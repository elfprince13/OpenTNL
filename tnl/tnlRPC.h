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

#ifndef _TNL_METHODDISPATCH_H_
#include "tnlMethodDispatch.h"
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

/// Base class for RPC events.
///
/// All declared RPC methods create subclasses of RPCEvent to send data across the wire
class RPCEvent : public NetEvent
{
public:
   MarshalledCall mCall;    ///< Call arguments marshalled into a buffer on the sender and represented for the stack on the receiver

   /// Constructor call from within the rpc<i>Something</i> method generated by the TNL_IMPLEMENT_RPC macro.
   RPCEvent(MethodArgList *aMarshaller, RPCGuaranteeType gType, RPCDirection dir);

   /// Copies the argument list into the event data.
   void marshallArguments();

   /// Returns the base address of the class member function pointer for the _remote version of the RPC function
   virtual void getFuncPtr(MethodPointer &m) = 0;

   /// Writes the marshalled data into the BitStream.
   void pack(EventConnection *ps, BitStream *bstream);

   /// Unmarshalls the data from the BitStream and stores it in mData.
   void unpack(EventConnection *ps, BitStream *bstream);

   /// Copies the unmarshalled member function arguments onto the stack and invokes the remote method.
   void process(EventConnection *ps);

   /// Returns true if this RPC is being sent to an object of the appropriate class.
   virtual bool checkClassType(Object *theObject) = NULL;
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

/// Macro used to declare the implementation of an RPC method on an EventConnection subclass.
///
/// The macro should be used in place of a member function parameter declaration,
/// with the body code (to be executed on the remote side of the RPC) immediately
/// following the TNL_IMPLEMENT_RPC macro call.
#define TNL_IMPLEMENT_RPC(className, name, args, groupMask, guaranteeType, eventDirection, rpcVersion) \
   extern TNL::MethodArgList RPC##className##name; \
   class RPC_##className##_##name : public TNL::RPCEvent { \
      public: \
      void (FN_CDECL className::*mFuncPtr) args; \
      TNL::U32 pad; \
      RPC_##className##_##name() : TNL::RPCEvent(&RPC##className##name, guaranteeType, eventDirection) \
         { mFuncPtr = &className::name##_remote; } \
      TNL_DECLARE_CLASS( RPC_##className##_##name ); \
      bool checkClassType(TNL::Object *theObject) { return dynamic_cast<className *>(theObject) != NULL; } \
      void getFuncPtr(TNL::MethodPointer &m) { m.v1=*((TNL::U32 *) &mFuncPtr); \
         if(sizeof(mFuncPtr) > sizeof(TNL::U32)) m.v2 = *(((TNL::U32 *) &mFuncPtr) + 1); \
         if(sizeof(mFuncPtr) > 2*sizeof(TNL::U32)) m.v3 = *(((TNL::U32 *) &mFuncPtr) + 2); } }; \
      TNL_IMPLEMENT_NETEVENT( RPC_##className##_##name, groupMask, rpcVersion ); \
      TNL::MethodArgList RPC##className##name (#className, #args); \
      void FN_CDECL className::name args { SAVE_PARAMS RPC_##className##_##name *theEvent = new RPC_##className##_##name; theEvent->marshallArguments(); postNetEvent(theEvent); } \
      TNL::NetEvent * FN_CDECL className::name##_construct args { SAVE_PARAMS RPC_##className##_##name *theEvent = new RPC_##className##_##name; theEvent->marshallArguments(); return theEvent; } \
      void FN_CDECL className::name##_test args { SAVE_PARAMS RPC_##className##_##name *ev = new RPC_##className##_##name; TNL::PacketStream ps; ev->marshallArguments(); ev->pack(this, &ps); ps.setBytePosition(0); ev->unpack(this, &ps); ev->process(this); } \
      void FN_CDECL className::name##_remote args
};

#endif
