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

RPCEvent::RPCEvent(MethodArgList *aMarshaller, RPCGuaranteeType gType, RPCDirection dir) :
      NetEvent((NetEvent::GuaranteeType) gType, (NetEvent::EventDirection) dir), 
      mCall(aMarshaller)
{
}

void RPCEvent::marshallArguments()
{
   mCall.marshall();
}

void RPCEvent::pack(EventConnection *ps, BitStream *bstream)
{
   bstream->writeBits(mCall.marshalledData.getBitPosition(), mCall.marshalledData.getBuffer());
   for(S32 i = 0; i < mCall.mSTEs.size(); i++)
      ps->packStringTableEntry(bstream, mCall.mSTEs[i]);
}

void RPCEvent::unpack(EventConnection *ps, BitStream *bstream)
{
   mCall.unmarshall(bstream);
   for(S32 i = 0; i < mCall.mSTEs.size(); i++)
      mCall.mSTEs[i] = ps->unpackStringTableEntry(bstream);
}

void RPCEvent::process(EventConnection *ps)
{
   if(!checkClassType(ps))
      return;

   MethodPointer p;
   getFuncPtr(p);
   mCall.dispatch(ps, &p);
}

};
