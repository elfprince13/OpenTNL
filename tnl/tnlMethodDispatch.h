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

#ifndef _TNL_METHODDISPATCH_H_
#define _TNL_METHODDISPATCH_H_

#ifndef _TNL_TYPES_H_
#include "tnlTypes.h"
#endif

#ifndef _TNL_VECTOR_H_
#include "tnlVector.h"
#endif

#ifndef _TNL_BITSTREAM_H_
#include "tnlBitStream.h"
#endif

#ifndef _TNL_NETSTRINGTABLE_H_
#include "tnlNetStringTable.h"
#endif

namespace TNL {

/// Manages an enumeration declared with TNL_DECLARE_ENUM
struct MethodEnum
{
   const char *mSymbol; ///< The name of the enumeration value

   U32 mValue;     ///< The value of the enumeration constant
   MethodEnum *mNext; ///< The next in global linked list of enums

   static MethodEnum *mLinkedList; ///< The head of the linked list of enums

   /// Constructor - called from statically allocated RPCEnums, using the declare macros
   MethodEnum(const char *symbol, U32 value)
   {
      mNext = mLinkedList;
      mLinkedList = this;
      mSymbol = symbol;
      mValue = value;
   }
};

/// Declares a global enumeration as usable as a template argument in an RPC declaration
#define TNL_DECLARE_ENUM(enumSymbol) namespace { TNL::MethodEnum gTNLEnum_##__LINE__(#enumSymbol, enumSymbol); };

struct MarshalledCall;
struct MethodArgInfo;

/// RPCArgList instances parse and evaluate the argument lists of declared RPC methods
struct MethodArgList
{
   /// MethodArgInfo tracks an individual parameter in an method parameter list
   struct MethodArgInfo {
      bool isVector;  ///< True if this is a vector of the specified type argument
      U32 argType;    ///< The type of this argument.
      U32 rangeStart; ///< Start of the U32 range, for TypeRangedU32s.
      U32 rangeEnd;   ///< End of the U32 range, for TypeRangedU32s.
      U32 bitCount;   ///< Bit size of this argument.
   };

   MethodArgList(const char *className, const char *argList);
   enum {
      MaxRPCDataSize = 4096,
      MaxOffsets = 1024,
      VectorSizeBitSize = 9,
      ByteBufferSizeBitSize = 10,
   };
   U32 getValue(const char *buffer); ///< Converts a text string into an enumerated value using the RPCEnum global linked list

   const char *argListString; ///< The original argument list for the RPC method, converted into a string in the macro
   const char *mClassName;    ///< Class name this RPC is a member of.
                              ///
                              ///  Used for resolving enums declared in this class

   Vector<MethodArgInfo> argList; ///< Vector of structures representing each argument in the RPC call

   S32 floatRegOffsets[13];    ///< offsets for reading the floating point values into registers
   void marshall(MarshalledCall *theCall); ///< Copies the arguments to the RPC method invocation into a BitStream buffer

   bool unmarshall(BitStream *stream, MarshalledCall *theEvent); ///< Reads the arguments from a BitStream buffer (packet) into an allocated buffer
                                        ///<
                                        ///<  Later we do some chicanery to stuff this back onto the sack for the method call.
};

struct MethodPointer
{
   U32 v1;
   U32 v2;
};

struct MarshalledCall
{
   BitStream marshalledData; ///< Bit compressed data marshalled from the call
   Vector<StringTableEntry> mSTEs; ///< Index list of StringTableEntry ids that are a part of this RPC
   ByteBuffer unmarshalledData; ///< Data from the call uncompressed into stack form
   MethodArgList *mMarshaller; ///< Proxy object responsible for marshalling and unmarshalling this call

   MarshalledCall(MethodArgList *marshaller) : marshalledData(NULL, 0)
   {
      mMarshaller = marshaller;
   }

   /// Reads the call's data from the stack and marshalls it into the internal bit stream
   void marshall()
   {
      mMarshaller->marshall(this);
   }

   /// Reads the bit compressed data from the input stream and unmarshalls it into unmarshalledData.
   /// If theStream is NULL it will unmarshall from its own marshalled data buffer.
   bool unmarshall(BitStream *theStream = NULL)
   {
      return mMarshaller->unmarshall(theStream ? theStream : &marshalledData, this);
   }

   /// Calls the method associated with this call on the given object and virtual function pointer.
   void dispatch(void *thisPointer, MethodPointer *method);
};

#if defined(TNL_SUPPORTS_VC_INLINE_X86_ASM)

extern void *gBasePtr;
#define SAVE_PARAMS __asm { lea eax, this }; __asm { mov gBasePtr, eax };

#elif defined(TNL_SUPPORTS_GCC_INLINE_X86_ASM )

extern void *gBasePtr;
#define SAVE_PARAMS gBasePtr = (void *) ((U8 *) __builtin_frame_address(0) + 8);

#elif defined(TNL_SUPPORTS_GCC_INLINE_PPC_ASM )
extern U32 gRegisterSaves[8 + 13 + 1];
#define SAVE_PARAMS __asm__ volatile ( \
  "mr r2, %0\n" \
  "stw r3, 0(r2) \n" \
  "stw r4, 4(r2) \n" \
  "stw r5, 8(r2) \n" \
  "stw r6, 12(r2) \n" \
  "stw r7, 16(r2) \n" \
  "stw r8, 20(r2) \n" \
  "stw r9, 24(r2) \n" \
  "stw r10, 28(r2) \n" \
  "stfs f1, 32(r2) \n" \
  "stfs f2, 36(r2) \n" \
  "stfs f3, 40(r2) \n" \
  "stfs f4, 44(r2) \n" \
  "stfs f5, 48(r2) \n" \
  "stfs f6, 52(r2) \n" \
  "stfs f7, 56(r2) \n" \
  "stfs f8, 60(r2) \n" \
  "stfs f9, 64(r2) \n" \
  "stfs f10, 68(r2) \n" \
  "stfs f11, 72(r2) \n" \
  "stfs f12, 76(r2) \n" \
  "stfs f13, 80(r2) \n" \
  "stw r1, 84(r2) \n" \
  : : "r" (gRegisterSaves) : "r2" );

#else
#error "Compiling RPC code without inline assembler support! You will need to implement RPCEvent::process() and co for your platform."
#endif

};

#endif