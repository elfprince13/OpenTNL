//-----------------------------------------------------------------------------------
//
//   Torque Network Library
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#include "tnl.h"
#include "tnlBitStream.h"
#include "tnlVector.h"
#include "tnlNetEvent.h"
#include "tnlRPC.h"
#include "tnlNetStringTable.h"
#include "tnlEventConnection.h"

namespace TNL {

void *gBasePtr = NULL;
void *gThisPtr = NULL;
void *gStackPtr = NULL;

static const char *typeNamesBase[] = {
"S8",
"U8",
"S16",
"U16",
"S32",
"U32",
"F32",
"SignedInt<",
"Int<",
"SignedFloat<",
"Float<",
"const char *",
"RangedU32<",
"bool",
"StringTableEntry",
"ByteBufferRef",
"IPAddressRef",
"IPAddress",
};

// Type sizes array.
// This array is used to allocate space for the data in vectors
// passed over RPCs.  Fields that have a zero value are not allowed
// to be passed in vectors.
static const U32 typeSizes[] = {
sizeof(S8),
sizeof(U8),
sizeof(S16),
sizeof(U16),
sizeof(S32),
sizeof(U32),
sizeof(F32),
sizeof(S32),
sizeof(U32),
sizeof(F32),
sizeof(F32),
sizeof(const char *),
sizeof(U32),
sizeof(bool),
sizeof(StringTableEntry),
0,
0,
sizeof(IPAddress),
};

RPCEnum *RPCEnum::mLinkedList = NULL;

U32 RPCArgList::getValue(const char *buffer)
{
   while(*buffer == ' ')
      buffer++;
   if(buffer[0] >= '0' && buffer[0] <= '9')
      return atoi(buffer);
   else
   {
      U32 len = 0;
      while((buffer[len] >= 'a' && buffer[len] <= 'z') ||
            (buffer[len] >= 'A' && buffer[len] <= 'Z') ||
            (buffer[len] == ':' || buffer[len] == '_'))
         len++;
      for(RPCEnum *walk = RPCEnum::mLinkedList; walk; walk = walk->mNext)
      {
         if(!strncmp(buffer, walk->mSymbol, len) && walk->mSymbol[len] == 0)
            return walk->mValue;
      }
   }
   TNLAssert(0, "Undeclared RPC enumeration!  Use TNL_DECLARE_RPC_ENUM.");
   return 0;
}

typedef StringTableEntry &StringTableEntryType;
RPCArgList::RPCArgList(const char *className, const char *anArgList)
{
   argListString = anArgList;
   mClassName = className;

   mNumStringTableEntries = 0;

   // first strip off the leading paren.
   const char *aptr = argListString + 1;
   U32 numFloats = 0;

   for(;;)
   {
      U32 type = 0;
      RPCArgInfo info;

      // remove leading spaces
      while(*aptr == ' ')
         aptr++;
      if(*aptr == ')')
         break;
      
      info.isVector = false;

      // this can let some errors through - for example we don't check that
      // the vector is passed as a reference, but it has to be.
      // Ideally we should do a simple lexer/parser for the argument
      // lists in flex/bison.  This would let us grow the parser a lot
      // more easily.
      if(!strncmp(aptr, "const Vector<", 13))
      {
         info.isVector = true;
         aptr += 13;
      }
      U32 len = 0;
      // grab the type off the arg list pointer:
      for(;type < RPCArgInfo::NumTypes; type++)
      {
         // figure out the base type:
         len = strlen(typeNamesBase[type]);
         if(!strncmp(aptr, typeNamesBase[type], len))
            break;
      }

      TNLAssert(type != RPCArgInfo::NumTypes, "Invalid type in RPC declaration.");
      TNLAssert(!info.isVector || typeSizes[type] != 0, "Invalid vector subtype in RPC declaration.");

      aptr += len;
      info.argType = type;

      // figure out the template arguments, if any:
      if(type == RPCArgInfo::TypeSignedInt || type == RPCArgInfo::TypeInt ||
         type == RPCArgInfo::TypeSignedFloat || type == RPCArgInfo::TypeFloat ||
         type == RPCArgInfo::TypeRangedU32)
      {
         const char *walk = aptr;
         aptr = strchr(aptr, '>') + 1;

         if(type == RPCArgInfo::TypeRangedU32)
         {
            info.rangeStart = getValue(walk);
            info.rangeEnd = getValue(strchr(walk, ',') + 1);
         }
         else
            info.bitCount = getValue(walk);
      }
      if (type == RPCArgInfo::TypeStringTableEntryRef)
         mNumStringTableEntries++;
      if(type == RPCArgInfo::TypeF32 || type == RPCArgInfo::TypeFloat || type == RPCArgInfo::TypeSignedFloat)
      {
         if(numFloats < 13)
         {
            floatRegOffsets[numFloats] = argList.size() * sizeof(U32);
            numFloats++;
         }
      }
      argList.push_back(info);
      // lop off the name of the var...
      aptr = strchr(aptr, ',');
      if(!aptr)
         break;
      aptr++;
   }
   if(numFloats < 13)
      floatRegOffsets[numFloats] = -1;
}

#ifdef TNL_CPU_PPC
U32 gRegisterSaves[8 + 13 + 1];
#endif

void RPCArgList::marshall(BitStream *bstream, RPCEvent *theEvent)
{
#if defined (TNL_CPU_PPC)
   U32 *intArg = gRegisterSaves;
   F32 *floatArg = (F32 *) (&gRegisterSaves[8]);
   void *stackPtr = (void *) gRegisterSaves[8 + 13];
   stackPtr = (void *) (*((U8 **) stackPtr) + 24); // advance past linkage area of stack
   intArg++; // get rid of the "this" pointer.
   bool floatInRegs = true;
#else
   U8 *ptr = (U8 *) gBasePtr;
   ptr += 4; // get rid of the return address and saved base ptr.
#endif
   U32 whichSTE = 0;
   for(S32 i = 0; i < argList.size(); i++)
   {
#if !defined (TNL_CPU_PPC)
      void *arg = ptr + sizeof(U32) * i;
      U32 *intArg = (U32 *) arg;
      F32 *floatArg = (F32 *) arg;
#else
      if(!floatInRegs)
         floatArg = ((F32 *) stackPtr) + i + 1; // the +1 skips the this pointer which is an arg, but not in the arg list.
#endif
      U32 count = 1;
      U32 *intArgSave = intArg;
      F32 *floatArgSave = floatArg;
      
      if(argList[i].isVector)
      {
         VectorRep *vector = *((VectorRep **) intArg);
         bstream->writeInt(vector->elementCount, VectorSizeBitSize);
         intArg = (U32 *) vector->array;
         floatArg = (F32 *) vector->array;
         count = vector->elementCount;
      }
      for(U32 j = 0; j < count; j++)
      {
         switch(argList[i].argType)
         {
            case RPCArgInfo::TypeS8:
               bstream->write(*((S8 *) intArg));
               break;
            case RPCArgInfo::TypeU8:
               bstream->write(*((U8 *) intArg));
               break;
            case RPCArgInfo::TypeS16:
               bstream->write(*((S16 *) intArg));
               break;
            case RPCArgInfo::TypeU16:
               bstream->write(*((U16 *) intArg));
               break;
            case RPCArgInfo::TypeS32:
               bstream->write(*((S32 *) intArg));
               break;
            case RPCArgInfo::TypeU32:
               bstream->write(*((U32 *) intArg));
               break;
            case RPCArgInfo::TypeF32:
               bstream->write(*((F32 *) floatArg));
               floatArg++;
               break;
            case RPCArgInfo::TypeSignedInt:
               bstream->writeSignedInt(*((S32 *) intArg), argList[i].bitCount);
               break;
            case RPCArgInfo::TypeInt:
               bstream->writeInt(*((U32 *) intArg), argList[i].bitCount);
               break;
            case RPCArgInfo::TypeSignedFloat:
               bstream->writeSignedFloat(*((F32 *) floatArg), argList[i].bitCount);
               floatArg++;
               break;
            case RPCArgInfo::TypeFloat:
               bstream->writeFloat(*((F32 *) floatArg), argList[i].bitCount);
               floatArg++;
               break;
            case RPCArgInfo::TypeString:
               bstream->writeString(*((const char **) intArg));
               break;
            case RPCArgInfo::TypeStringTableEntryRef:
               theEvent->mSTEs.push_back(*((StringTableEntry *) intArg));
               break;
            case RPCArgInfo::TypeRangedU32:
               bstream->writeRangedU32(*((U32 *) intArg), argList[i].rangeStart, argList[i].rangeEnd);
               break;
            case RPCArgInfo::TypeBool:
               bstream->writeFlag(*((bool *) intArg));
               break;
            case RPCArgInfo::TypeByteBufferPtr:
               {
               ByteBuffer *buffer = *((ByteBuffer **) intArg);
               bstream->writeInt(buffer->getBufferSize(), ByteBufferSizeBitSize);
               bstream->write(buffer->getBufferSize(), buffer->getBuffer());
               }
               break;
            case RPCArgInfo::TypeIPAddressRef:
               bstream->write((*((IPAddress **) intArg))->netNum);
               bstream->write((*((IPAddress **) intArg))->port);
               break;
            case RPCArgInfo::TypeIPAddress:
               bstream->write(((IPAddress *) intArg)->netNum);
               bstream->write(((IPAddress *) intArg)->port);
               break;
         }
         if(count != 1)
         {
            *((U8 **) &intArg) += typeSizes[argList[i].argType];
            // float args already get incremented above
            //*((U8 **) &floatArg) += typeSizes[argList[i].argType];
         }
      }
      if(argList[i].isVector)
      {
         intArg = intArgSave;
         floatArg = floatArgSave;
      }
      intArg++;
#ifdef TNL_CPU_PPC
      if(i == 6) // we're on our 8th register... start reading ints from the stack:
      {
         intArg = ((U32 *) stackPtr) + 8; //  skip 8 registers.
      }
      if(floatArg == (F32 *) &gRegisterSaves[8+13]) // we're on our 13th register... start reading floats from the stack:
         floatInRegs = false;
#endif
   }
}

static U8 rpcReadData[RPCArgList::MaxRPCDataSize];
static U32 rpcReadOffsets[RPCArgList::MaxOffsets];
static U8 *rpcSTEOffsets[RPCArgList::MaxOffsets];
static U32 rpcNumSTEs;
static U32 rpcNumOffsets;

inline U8 *allocSpace(U8 **offsetDest, U32 &currentSize, U32 requestSize)
{
   if(rpcNumOffsets >= RPCArgList::MaxOffsets)
      return NULL;

   currentSize = fourByteAlign(currentSize);
   rpcReadOffsets[rpcNumOffsets++] = ((U8 *) offsetDest) - rpcReadData;

   if(requestSize + currentSize > RPCArgList::MaxRPCDataSize)
      return NULL;

   U8 *ret = rpcReadData + currentSize;
   *offsetDest = ret;

   currentSize += requestSize;
   return ret;   
}

void *RPCArgList::unmarshall(BitStream *bstream, EventConnection *ps, RPCEvent *theEvent)
{
   rpcNumOffsets = 0;
   rpcNumSTEs = 0;
   U32 currentSize = sizeof(U32) * argList.size();
#ifdef TNL_CPU_PPC
   currentSize = getMax(currentSize, U32(28));
#endif
   U32 whichSTE = 0;
   for(S32 i = 0; i < argList.size(); i++)
   {
      U8 *arg = (rpcReadData + sizeof(U32) * i);
      U32 count = 1;
      if(argList[i].isVector)
      {
         VectorRep *vector = (VectorRep *) allocSpace((U8 **) arg, currentSize, sizeof(VectorRep));
         if(!vector)
            goto errorOut;

         vector->arraySize = bstream->readInt(VectorSizeBitSize);
         vector->elementCount = vector->arraySize;

         vector->array = allocSpace(&vector->array, currentSize, vector->elementCount * typeSizes[argList[i].argType]);
         if(!vector->array)
            goto errorOut;
         *((VectorRep **) arg) = vector;
         count = vector->elementCount;
         arg = vector->array;
      }
      for(U32 j = 0; j < count; j++)
      {
         switch(argList[i].argType)
         {
            case RPCArgInfo::TypeS8:
               bstream->read((S8 *) arg);
               break;
            case RPCArgInfo::TypeU8:
               bstream->read((U8 *) arg);
               break;
            case RPCArgInfo::TypeS16:
               bstream->read((S16 *) arg);
               break;
            case RPCArgInfo::TypeU16:
               bstream->read((S16 *) arg);
               break;
            case RPCArgInfo::TypeS32:
               bstream->read((S32 *) arg);
               break;
            case RPCArgInfo::TypeU32:
               bstream->read((U32 *) arg);
               break;
            case RPCArgInfo::TypeF32:
               bstream->read((F32 *) arg);
               break;
            case RPCArgInfo::TypeSignedInt:
               *((S32 *) arg) = bstream->readSignedInt(argList[i].bitCount);
               break;
            case RPCArgInfo::TypeInt:
               *((U32 *) arg) = bstream->readInt(argList[i].bitCount);
               break;
            case RPCArgInfo::TypeSignedFloat:
               *((F32 *) arg) = bstream->readSignedFloat(argList[i].bitCount);
               break;
            case RPCArgInfo::TypeFloat:
               *((F32 *) arg) = bstream->readFloat(argList[i].bitCount);
               break;
            case RPCArgInfo::TypeString:
               {
               char **stringPtr = (char **) arg;
               *stringPtr = (char *) allocSpace((U8 **) arg, currentSize, 256);
               if(!*stringPtr)
                  goto errorOut;
               bstream->readString(*stringPtr);
               // back off the end pointer.
               currentSize -= (255 - strlen(*stringPtr));
               }
               break;
            case RPCArgInfo::TypeStringTableEntryRef:
               if(rpcNumSTEs >= RPCArgList::MaxOffsets)
                  goto errorOut;
               rpcSTEOffsets[rpcNumSTEs++] = arg;
               break;
            case RPCArgInfo::TypeRangedU32:
               *((U32 *) arg) = bstream->readRangedU32(argList[i].rangeStart, argList[i].rangeEnd);
               break;
            case RPCArgInfo::TypeBool:
               *((bool *) arg) = bstream->readFlag();
               break;
            case RPCArgInfo::TypeByteBufferPtr:
               {
               ByteBuffer *buffer = (ByteBuffer *) allocSpace((U8 **) arg, currentSize, sizeof(ByteBuffer));
               if(!buffer)
                  goto errorOut;
               buffer->mOwnsMemory = false;
               buffer->mBufSize = bstream->readInt(ByteBufferSizeBitSize);
               buffer->mDataPtr = (U8 *) allocSpace(&buffer->mDataPtr, currentSize, buffer->mBufSize);
               if(!buffer->mDataPtr)
                  goto errorOut;
               bstream->read(buffer->mBufSize, buffer->mDataPtr);
               *((ByteBuffer **) arg) = buffer;
               }
               break;
            case RPCArgInfo::TypeIPAddressRef:
               {
               IPAddress *ip = (IPAddress *) allocSpace((U8 **) arg, currentSize, sizeof(IPAddress));
               if(!ip)
                  goto errorOut;
               bstream->read(&ip->netNum);
               bstream->read(&ip->port);
               *((IPAddress **) arg) = (IPAddress *) ip;
               }
               break;
            case RPCArgInfo::TypeIPAddress:
               bstream->read(&((IPAddress *) arg)->netNum);
               bstream->read(&((IPAddress *) arg)->port);
               break;
         }
         *((U8 **) &arg) += typeSizes[argList[i].argType];
      }
   }
   for(U32 i = 0; i < rpcNumSTEs; i++)
   {
      StringTableEntry ste = ps->unpackStringTableEntry(bstream);
      theEvent->mSTEs.push_back(ste);
      *((U32 *) rpcSTEOffsets[i]) = ste.getIndex();
   }

   U8 *data;
   data = (U8 *)malloc(currentSize);
   U32 delta;
   delta = data - rpcReadData;
   for(U32 i = 0; i < rpcNumOffsets; i++)
   {
      *((U8 **) (rpcReadData + rpcReadOffsets[i])) += delta;
   }

   memcpy(data, rpcReadData, currentSize);
   return data;
errorOut:
   return NULL;
}


RPCEvent::RPCEvent(RPCArgList *aMarshaller, RPCGuaranteeType gType, RPCDirection dir) :
      NetEvent((NetEvent::GuaranteeType) gType, (NetEvent::EventDirection) dir), mData(NULL), mMarshaller(aMarshaller)
{
}

RPCEvent::~RPCEvent()
{
   free(mData);
}

void RPCEvent::marshallArguments()
{
   PacketStream s;
   mMarshaller->marshall(&s, this);
   mBitCount = s.getBitPosition();
   mData = malloc(s.getBytePosition());
   memcpy(mData, s.getBuffer(), s.getBytePosition());
}

void RPCEvent::pack(EventConnection *ps, BitStream *bstream)
{
   bstream->writeBits(mBitCount, mData);
   for(S32 i = 0; i < mSTEs.size(); i++)
      ps->packStringTableEntry(bstream, mSTEs[i]);
}

void RPCEvent::unpack(EventConnection *ps, BitStream *bstream)
{
   mData = mMarshaller->unmarshall(bstream, ps, this);
   if(!mData)
   {
      ps->setLastError("Invalid packet.");
   }
}

void RPCEvent::process(EventConnection *ps)
{
   U32 func;
   U32 funcOffset;
   getFuncPtr(func, funcOffset);
   void *args = mData;
   void *thisPtr = getThisPointer(ps);

#ifdef TNL_CPU_X86
   S32 sz = mMarshaller->argList.size() * sizeof(U32);
   U32 saveESP;
#endif
   //   CallMethod(ps, data);

#if defined(TNL_SUPPORTS_VC_INLINE_X86_ASM)
   _asm
   {
      push  esi
      push  edi
      push  ecx
      push  edx
      push  ebx
      mov   saveESP, esp

      mov   ecx, sz       // get size of buffer
      mov   esi, args     // get buffer
      sub   esp, ecx      // allocate stack space
      mov   edi, esp      // start of destination stack frame
      shr   ecx, 2        // make it dwords
      rep   movsd         // copy it
      mov   ecx, thisPtr  // set "this"
      push  ecx
      call  [func]        // call the function
      mov   esp, saveESP

      pop   ebx
      pop   edx
      pop   ecx
      pop   edi
      pop   esi
   }

#elif defined(TNL_SUPPORTS_GCC_INLINE_X86_ASM)
      U8 *funcPtr = ((U8 *) thisPtr) + funcOffset;
      funcPtr = *((U8 **) funcPtr);
      funcPtr += func;
      funcPtr--;
      funcPtr = *((U8 **) funcPtr);

   asm(
    "mov %%esp, %0 \n"

    "mov %1, %%ecx \n"      // get size of buffer
    "mov %2, %%esi \n"      // get buffer
    "sub   %%ecx, %%esp \n" // allocate stack space
    "mov   %%esp, %%edi \n" //  start of destination stack frame
    "shr   $2, %%ecx \n"    // make it dwords
    "rep\nmovsl \n"         // copy it
    "mov   %4, %%ecx \n"    // set "this"
    "push %%ecx \n"         // gcc expects this pointer to be on the stack
    "call *%3 \n"           // call the function

    "mov %0, %%esp \n"

    : /* nothing out */
    : "m" (saveESP), "m" (sz), "m" (args), "m" (funcPtr), "m" (thisPtr)
    : "ebx", "edx", "ecx", "edi", "esi"
    );
#elif defined (TNL_SUPPORTS_GCC_INLINE_PPC_ASM)
   register S32 *floatOffsets = mMarshaller->floatRegOffsets;

   S32 argAreaSize = mMarshaller->argList.size() * sizeof(U32);
   if(argAreaSize < 28)
      argAreaSize = 28;

   S32 copySize = 28 - argAreaSize; // negative - stack grows down

      // load up the floating point registers
   asm(" \n"
      "mflr r0\n"
      "stw r0, 8(r1)\n"           // save off link register
      "mr r9, %4\n"               // load up the copy size
      "addi r9, r9, -24-32-4\n"   // 24 for linkage area, 32 for register arguments, 4 for r20
      "stw r20, -4(r1)\n"         // save off r20 into the red zone
      "mr r20, r9\n"              // save the grow amount

      "subfic r9, r9, -24-32-4\n" // compute byte total to copy to the stack
      "mr r10, %1\n"
      "addi r10, r10, 28\n"
      "sub r12, r1, r9\n"         // write offset from stack (into red zone)
      "addi r12, r12, -4\n"       // skip past r20 save area.
      "li r2, 0\n"

      "stackCopyLoop:\n"
      "cmpw r2, r9\n"
      "beq stackCopyDone\n"
      "lwzx r3, r2, r10\n"
      "stwx r3, r2, r12\n"
      "addi r2, r2, 4\n"          // increment the loop counter...
      "b stackCopyLoop\n"
      "stackCopyDone:\n"

      "mr r10, %1\n"
      "mr r3, %3\n"
      "mr r12, %2\n"
      "mr r9, %0\n"

      "stwux r1, r1, r20\n"       // create some stack space for us to work in

      "lwz r2, 0(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f1, r2, r10\n"

      "lwz r2, 4(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f2, r2, r10\n"

      "lwz r2, 8(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f3, r2, r10\n"

      "lwz r2, 12(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f4, r2, r10\n"

      "lwz r2, 16(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f5, r2, r10\n"

      "lwz r2, 20(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f6, r2, r10\n"

      "lwz r2, 24(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f7, r2, r10\n"

      "lwz r2, 28(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f8, r2, r10\n"

      "lwz r2, 32(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f9, r2, r10\n"

      "lwz r2, 36(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f10, r2, r10\n"

      "lwz r2, 40(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f11, r2, r10\n"

      "lwz r2, 44(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f12, r2, r10\n"

      "lwz r2, 48(r9)\n"
      "subic. r2, r2, 0\n"
      "blt floatsDone\n"
      "lfsx f13, r2, r10\n"

      "floatsDone:\n"

      // load up the virtual function pointer
      "mr r4, %5\n"
      // r4 has funcOffset, r12 has the function index
      "add r4, r4, r3\n"
      "lwz r4, 0(r4)\n"
      "add r4, r4, r12\n"
      "addi r4, r4, -1\n"
      "lwz r12, 0(r4)\n"

      "lwz r4, 0(r10)\n"
      "lwz r5, 4(r10)\n"
      "lwz r6, 8(r10)\n"
      "lwz r7, 12(r10)\n"
      "lwz r8, 16(r10)\n"
      "lwz r9, 20(r10)\n"
      "lwz r10, 24(r10)\n"
      "mtctr r12\n"
      "bctrl\n"
      "sub r1, r1, r20\n"
      "lwz r20, -4(r1)\n"         //addic r1, r1, 424\n"
      "lwz r0, 8(r1)\n"
      "mtlr r0\n"

      : : "r" (floatOffsets), "r" (args), "r" (func), "r" (thisPtr), "r" (copySize), "r" (funcOffset) : "r2", "r9", "r10", "r12", "r3", "r0");
#endif
}

};
