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

#ifndef _TNL_RPC_H_
#define _TNL_RPC_H_

#ifndef _TNL_NETEVENT_H_
#include "tnlNetEvent.h"
#endif

namespace TNL {

/*! @page rpcdesc RPC in the Torque Network Library

The Torque Network Library has a powerful, yet simple to use Remote
Procedure Call framework for passing information through a network
connection.  Subclasses of EventConnection and NetObject can declare
member functions using the RPC macros so that when called the function 
arguments are sent to the remote EventConnection or NetObject(s) associated
with that object.

For example, suppose you have a connection class called SimpleEventConnection:
@code
class SimpleEventConnection : public EventConnection
{
public:
   TNL_DECLARE_RPC(rpcPrintHelloWorld, ());
};

TNL_IMPLEMENT_RPC(SimpleEventConnection, rpcPrintHelloWorld, (),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirAny, 0)
{
   printf("Hello, World!");
}
 
 ...
void somefunction(SimpleEventConnection *connection)
{
   connection->rpcPrintHelloWorld();
}
@endcode

In this example the class SimpleEventConnection is declared to have
a single RPC method named rpcPrintHelloWorld.  The TNL_DECLARE_RPC macro
can just be viewed as a different way of declaring a class's member functions,
with the name as the first argument and the parenthesized parameter list
as the second.  Since RPC calls execute on a remote host, they never have
a return value - athough a second RPC could be declared to pass messages
in the other direction.

The body of the RPC method is declared using the TNL_IMPLEMENT_RPC macro,
which has some additional arguments, including which NetClassMask the RPC is valid
in, what level of data guarantee it uses, the direction it is allowed to be called
in the connection and a version number.  The body of the function, which
in this case prints the message "Hello, World!" to stdout, is executed on
the remote host from which the method was originally invoked.

As the somefunction code demonstrates, RPC's are invoked in the same way
as any other member function in the class.

RPCs behave like virtual functions in that their bodies can be overridden
in subclasses that want to implement new behavior for the message.  The class
declaration for an overridden RPC should include the TNL_DECLARE_RPC_OVERRIDE
macro used for each method that will be redefined.  The TNL_IMPLEMENT_RPC_OVERRIDE
macro should be used outside the declaration of the class to implement
the body of the new RPC.

RPC methods can take the following types as arguments:
 - S8, U8
 - S16, U16
 - S32, U32
 - F32
 - Int<>
 - SignedInt<>
 - Float<>
 - SignedFloat<>
 - RangedU32<>
 - bool
 - const char *
 - StringTableEntry
 - ByteBufferRef
 - IPAddressRef
 - Vector<> of all the preceding types except ByteBufferRef and IPAddressRef

The Int, SignedInt, Float, SignedFloat and RangedU32 template types use the
template parameter(s) to specify the number of bits necessary to transmit that
variable across the network.  For example:

@code
  ...
 TNL_DECLARE_RPC(someTestFunction, (Int<4> fourBitInt, SignedFloat<7> aFloat, 
    RangedU32<100, 199> aRangedU32);
  ...
@endcode
The preceding RPC method would use 4 + 7 + 7 = 18 bits to transmit the arguments
to the function over the network, not including the RPC event overhead.
*/

/// NetType serves as a base class for all bit-compressed versions of
/// the base types that can be transmitted using TNL's RPC mechanism.
/// In general, the type names are self-explanatory, providing simple
/// wrappers on the original base types.  The template argument for bit
/// counts or numeric ranges is necessary because TNL parses the actual
/// function prototype as a string in order to determine how many bits
/// to use for each RPC parameter.
///
/// Template parameters to the NetType templates can be either integer
/// constants or enumeration values.  If enumeration values are used,
/// the TNL_DECLARE_RPC_ENUM or TNL_DECLARE_RPC_MEM enum macros must
/// be used to register the enumerations with the RPC system.
struct NetType {

};

/// Unsigned integer bit-level RPC template wrapper.
///
/// When an Int<X> is in the parameter list for an RPC method, that parameter will
/// be transmitted using X bits.
template<U32 bitCount> struct Int : NetType
{
   U32 value;
   Int(U32 val=0) { value = val; }
   operator U32() const { return value; }
   U32 getPrecisionBits() { return bitCount; }
};

/// Signed integer bit-level RPC template wrapper.
///
/// When a SignedInt<X> is in the parameter list for an RPC method, that parameter will
/// be transmitted using X bits.
template<U32 bitCount> struct SignedInt : NetType
{
   S32 value;
   SignedInt(S32 val=0) { value = val; }
   operator S32() const { return value; }
   U32 getPrecisionBits() { return bitCount; }
};

/// Floating point 0...1 value bit-level RPC template wrapper.
///
/// When a Float<X> is in the parameter list for an RPC method, that parameter will
/// be transmitted using X bits.
template<U32 bitCount> struct Float : NetType
{
   F32 value;
   Float(F32 val=0) { value = val; }
   operator F32() const { return value; }
   U32 getPrecisionBits() { return bitCount; }
};

/// Floating point -1...1 value bit-level RPC template wrapper.
///
/// When a SignedFloat<X> is in the parameter list for an RPC method, that parameter will
/// be transmitted using X bits.
template<U32 bitCount> struct SignedFloat : NetType
{
   F32 value;
   SignedFloat(F32 val=0) { value = val; }
   operator F32() const { return value; }
   U32 getPrecisionBits() { return bitCount; }
};

/// Unsigned ranged integer bit-level RPC template wrapper.
///
/// The RangedU32 is used to specify a range of valid values for the parameter
/// in the parameter list for an RPC method.
template<U32 rangeStart, U32 rangeEnd> struct RangedU32 : NetType
{
   U32 value;
   RangedU32(U32 val=rangeStart) { TNLAssert(value >= rangeStart && value <= rangeEnd, "Out of range value!"); value = val; }
   operator U32() const { return value; }
};

/// RPCArgInfo tracks an individual parameter in an RPC method parameter list
struct RPCArgInfo {
   enum Type {
      TypeS8,
      TypeU8,
      TypeS16,
      TypeU16,
      TypeS32,
      TypeU32,
      TypeF32,
      TypeSignedInt,
      TypeInt,
      TypeSignedFloat,
      TypeFloat,
      TypeString,
      TypeRangedU32,
      TypeBool,
      TypeStringTableEntryRef,
      TypeByteBufferPtr,
      TypeIPAddressRef,
      TypeIPAddress,
      NumTypes,
   };
   bool isVector;  ///< True if this is a vector of the specified type argument
   U32 argType;    ///< The type of this argument.
   U32 rangeStart; ///< Start of the U32 range, for TypeRangedU32s.
   U32 rangeEnd;   ///< End of the U32 range, for TypeRangedU32s.
   U32 bitCount;   ///< Bit size of this argument.
};

/// Manages an enumeration declared with TNL_DECLARE_RPC_ENUM or TNL_DECLARE_RPC_MEM_ENUM
struct RPCEnum
{
   const char *mSymbol; ///< The name of the enumeration value

   U32 mValue;     ///< The value of the enumeration constant
   RPCEnum *mNext; ///< The next in global linked list of enums

   static RPCEnum *mLinkedList; ///< The head of the linked list of enums

   /// Constructor - called from statically allocated RPCEnums, using the declare macros
   RPCEnum(const char *symbol, U32 value)
   {
      mNext = mLinkedList;
      mLinkedList = this;
      mSymbol = symbol;
      mValue = value;
   }
};

/// Declares a global enumeration as usable as a template argument in an RPC declaration
#define TNL_DECLARE_RPC_ENUM(enumSymbol) namespace { TNL::RPCEnum enm##enumSymbol(#enumSymbol, enumSymbol); };

/// Declares a class member enumeration as usable as a template argument in an RPC declaration
#define TNL_DECLARE_RPC_MEM_ENUM(enumClass, enumSymbol) namespace  { TNL::RPCEnum enm##enumClass##enumSymbol(#enumClass "::" #enumSymbol, enumClass::enumSymbol); };

class RPCEvent;

/// RPCArgList instances parse and evaluate the argument lists of declared RPC methods
struct RPCArgList
{
   RPCArgList(const char *className, const char *argList);
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

   Vector<RPCArgInfo> argList; ///< Vector of structures representing each argument in the RPC call

   U32 mNumStringTableEntries; ///< The number of arguments that are string table entries

   S32 floatRegOffsets[13];    ///< offsets for reading the floating point values into registers
   void marshall(BitStream *stream, RPCEvent *theEvent); ///< Copies the arguments to the RPC method invocation into a BitStream buffer

   void *unmarshall(BitStream *stream, EventConnection *ps, RPCEvent *theEvent); ///< Reads the arguments from a BitStream buffer (packet) into an allocated buffer
                                        ///<
                                        ///<  Later we do some chicanery to stuff this back onto the sack for the method call.
};

/// Enumeration for valid directions that RPC messages can travel
enum RPCDirection {
   RPCDirAny            = NetEvent::DirAny,           ///< This RPC can be sent from the server or the client
   RPCDirServerToClient = NetEvent::DirServerToClient,///< This RPC can only be sent from the server to the client
   RPCDirClientToServer = NetEvent::DirClientToServer,///< This RPC can only be sent from the client to the server
};

/// Type of data guarantee this RPC should use
enum RPCGuaranteeType {
   RPCGuaranteedOrdered = NetEvent::GuaranteedOrdered, ///< RPC event delivery is guaranteed and will be processed in the order it was sent relative to other ordered events and RPCs
   RPCGuaranteed        = NetEvent::Guaranteed,        ///< RPC event delivery is guaranteed and will be processed in the order it was received
   RPCUnguaranteed      = NetEvent::Unguaranteed       ///< Event delivery is not guaranteed - however, the event will remain ordered relative to other unguaranteed events
};

class StringTableEntry;

/// Base class for RPC events.
///
/// All declared RPC methods create subclasses of RPCEvent to send data across the wire
class RPCEvent : public NetEvent
{
public:
   void *mData;             ///< Argument data copied from the stack on the initiator, and copied to the stack on the receiver.
   U32 mBitCount;           ///< Total number of bits to be written into the BitStream on pack()
   RPCArgList *mMarshaller; ///< Object describing the argument list to call
   Vector<StringTableEntry> mSTEs; ///< Index list of StringTableEntry ids that are a part of this RPC

   /// Constructor call from within the rpc<i>Something</i> method generated by the TNL_IMPLEMENT_RPC macro.
   RPCEvent(RPCArgList *aMarshaller, RPCGuaranteeType gType, RPCDirection dir);
   ~RPCEvent();

   /// Copies the argument list into the event data.
   void RPCEvent::marshallArguments();

   /// Returns the base address of the class member function pointer for the _remote version of the RPC function
   virtual void getFuncPtr(U32 &v1, U32 &v2) = 0;

   /// Writes the marshalled data into the BitStream.
   void pack(EventConnection *ps, BitStream *bstream);

   /// Unmarshalls the data from the BitStream and stores it in mData.
   void unpack(EventConnection *ps, BitStream *bstream);

   /// Copies the unmarshalled member function arguments onto the stack and invokes the remote method.
   void process(EventConnection *ps);

   /// Returns the "this" pointer that the remote method should be called on
   virtual void *getThisPointer(EventConnection *ps) { return (void *) ps; }
};

/// Declares an RPC method within a class declaration.  Creates two method prototypes - one for the host side of the RPC call, and one for the receiver, which performs the actual method.
#define TNL_DECLARE_RPC(name, args) virtual void FN_CDECL name args; void FN_CDECL name##_test args; virtual TNL::NetEvent * FN_CDECL name##_construct args; virtual void FN_CDECL name##_remote args

/// Declares an override to an RPC method declared in a parent class.
#define TNL_DECLARE_RPC_OVERRIDE(name, args) void FN_CDECL name##_remote args

/// Macro used to declare the body of an overridden RPC method.
#define TNL_IMPLEMENT_RPC_OVERRIDE(className, name, args) \
   void FN_CDECL className::name##_remote args

/// Constructs a NetEvent that will represent the specified RPC invocation.  This
/// macro is used to construct a single RPC that can then be posted to multiple
/// connections, instead of allocating an RPCEvent for each connection.
#define TNL_RPC_CONSTRUCT_NETEVENT(object, rpcMethod, args) (object)->rpcMethod##_construct args

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

/// Macro used to declare the implementation of an RPC method on an EventConnection subclass.
///
/// The macro should be used in place of a member function parameter declaration,
/// with the body code (to be executed on the remote side of the RPC) immediately
/// following the TNL_IMPLEMENT_RPC macro call.
#define TNL_IMPLEMENT_RPC(className, name, args, groupMask, guaranteeType, eventDirection, rpcVersion) \
   extern TNL::RPCArgList RPC##className##name; \
   class RPC_##className##_##name : public RPCEvent { \
      public: \
      void (FN_CDECL className::*mFuncPtr) args; \
      U32 pad; \
      RPC_##className##_##name() : RPCEvent(&RPC##className##name, guaranteeType, eventDirection) \
         { mFuncPtr = &className::name##_remote; } \
      TNL_DECLARE_CLASS( RPC_##className##_##name ); \
      void getFuncPtr(U32 &v1, U32 &v2) { v1=*((U32 *) &mFuncPtr); v2 = *(((U32 *) &mFuncPtr) + 1); } }; \
      TNL_IMPLEMENT_NETEVENT( RPC_##className##_##name, groupMask, rpcVersion ); \
      TNL::RPCArgList RPC##className##name (#className, #args); \
      void FN_CDECL className::name args { SAVE_PARAMS RPC_##className##_##name *theEvent = new RPC_##className##_##name; theEvent->marshallArguments(); postNetEvent(theEvent); } \
      TNL::NetEvent * FN_CDECL className::name##_construct args { SAVE_PARAMS RPC_##className##_##name *theEvent = new RPC_##className##_##name; theEvent->marshallArguments(); return theEvent; } \
      void FN_CDECL className::name##_test args { SAVE_PARAMS RPC_##className##_##name *ev = new RPC_##className##_##name; PacketStream ps; ev->marshallArguments(); ev->pack(this, &ps); ps.setBytePosition(0); ev->unpack(this, &ps); ev->process(this); } \
      void FN_CDECL className::name##_remote args
};

#endif
