//-----------------------------------------------------------------------------------
//
//   Torque Network Library - ZAP example multiplayer vector graphics space game
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

#include "gameObject.h"
#include "controlObjectConnection.h"

namespace Zap
{

ControlObjectConnection::ControlObjectConnection()
{
   highSendIndex[0] = 0;
   highSendIndex[1] = 0;
   highSendIndex[2] = 0;
   mLastClientControlCRC = 0;
   firstMoveIndex = 1;
}

void ControlObjectConnection::setControlObject(GameObject *theObject)
{
   if(controlObject.isValid())
      controlObject->setControllingClient(NULL);

   controlObject = theObject;
   setScopeObject(theObject);

   if(theObject)
      theObject->setControllingClient((GameConnection *) this);
}

void ControlObjectConnection::packetReceived(PacketNotify *notify)
{
   for(; firstMoveIndex < ((GamePacketNotify *) notify)->firstUnsentMoveIndex; firstMoveIndex++)
      pendingMoves.erase(U32(0));
   mServerPosition = ((GamePacketNotify *) notify)->lastControlObjectPosition;
   Parent::packetReceived(notify);
}

U32 ControlObjectConnection::getControlCRC()
{
   PacketStream stream;
   GameObject *co = getControlObject();

   if(!co)
      return 0;

   stream.writeInt(getGhostIndex(co), GhostConnection::GhostIdBitSize);
   co->writeControlState(&stream);
   stream.zeroToByteBoundary();
   return stream.calculateCRC(0, stream.getBytePosition());   
}

void ControlObjectConnection::writePacket(BitStream *bstream, PacketNotify *notify)
{
   if(isConnectionToServer())
   {
      U32 firstSendIndex = highSendIndex[0];
      if(firstSendIndex < firstMoveIndex)
         firstSendIndex = firstMoveIndex;

      bstream->write(getControlCRC());

      bstream->write(firstSendIndex);
      U32 skipCount = firstSendIndex - firstMoveIndex;
      U32 moveCount = pendingMoves.size() - skipCount;

      bstream->writeRangedU32(moveCount, 0, MaxPendingMoves);
      Move dummy;
      Move *lastMove = &dummy;
      for(S32 i = skipCount; i < pendingMoves.size(); i++)
      {
         pendingMoves[i].pack(bstream, lastMove);
         lastMove = &pendingMoves[i];
      }
      ((GamePacketNotify *) notify)->firstUnsentMoveIndex = firstMoveIndex + pendingMoves.size();
      if(controlObject.isValid())
         ((GamePacketNotify *) notify)->lastControlObjectPosition = controlObject->getActualPos();

      highSendIndex[0] = highSendIndex[1];
      highSendIndex[1] = highSendIndex[2];
      highSendIndex[2] = ((GamePacketNotify *) notify)->firstUnsentMoveIndex;
   }
   else
   {
      S32 ghostIndex = -1;
      if(controlObject.isValid())
      {
         ghostIndex = getGhostIndex(controlObject);
         mServerPosition = controlObject->getActualPos();
      }

      // we only compress points relative if we know that the
      // remote side has a copy of the control object already
      mCompressPointsRelative = bstream->writeFlag(ghostIndex != -1);

      if(bstream->writeFlag(getControlCRC() != mLastClientControlCRC))
      {
         if(ghostIndex != -1)
         {
            bstream->writeInt(ghostIndex, GhostConnection::GhostIdBitSize);
            controlObject->writeControlState(bstream);
         }
      }
   }
   Parent::writePacket(bstream, notify);
}

void ControlObjectConnection::readPacket(BitStream *bstream)
{
   if(isConnectionToClient())
   {
      bstream->read(&mLastClientControlCRC);

      U32 firstMove;
      bstream->read(&firstMove);
      U32 count = bstream->readRangedU32(0, MaxPendingMoves);

      Move theMove;
      for(; firstMove < firstMoveIndex && count > 0; firstMove++)
      {
         count--;
         theMove.unpack(bstream);
      }
      for(; count > 0; count--)
      {
         theMove.unpack(bstream);
         // process the move, including crediting time to the client
         // and all that joy.
         if(controlObject.isValid())
         {
            controlObject->setCurrentMove(theMove);
            controlObject->idle(GameObject::ServerIdleControlFromClient);
         }
         firstMoveIndex++;
      }
   }
   else
   {
      bool controlObjectValid = bstream->readFlag();

      mCompressPointsRelative = controlObjectValid;

      // CRC mismatch...
      if(bstream->readFlag())
      {
         if(controlObjectValid)
         {
            U32 ghostIndex = bstream->readInt(GhostConnection::GhostIdBitSize);
            controlObject = (GameObject *) resolveGhost(ghostIndex);
            controlObject->readControlState(bstream);
            mServerPosition = controlObject->getActualPos();

            for(S32 i = 0; i < pendingMoves.size(); i++)
            {
               Move theMove = pendingMoves[i];
               theMove.prepare();
               controlObject->setCurrentMove(theMove);
               controlObject->idle(GameObject::ClientIdleControlReplay);
            }
            controlObject->controlMoveReplayComplete();
         }
         else
            controlObject = NULL;
      }
   }
   Parent::readPacket(bstream);
}

void ControlObjectConnection::writeCompressedPoint(Point &p, BitStream *stream)
{
   if(!mCompressPointsRelative)
   {
      stream->write(p.x);
      stream->write(p.y);
      return;
   }

   Point delta = p - mServerPosition;
   S32 dx = S32(delta.x + Game::PlayerHorizScopeDistance);
   S32 dy = S32(delta.y + Game::PlayerVertScopeDistance);

   S32 maxx = Game::PlayerHorizScopeDistance * 2;
   S32 maxy = Game::PlayerVertScopeDistance * 2;

   if(stream->writeFlag(dx >= 0 && dx <= maxx && dy >= 0 && dy <= maxy))
   {
      stream->writeRangedU32(dx, 0, maxx);
      stream->writeRangedU32(dy, 0, maxy);
   }
   else
   {
      stream->write(p.x);
      stream->write(p.y);
   }
}

void ControlObjectConnection::readCompressedPoint(Point &p, BitStream *stream)
{
   if(!mCompressPointsRelative)
   {
      stream->read(&p.x);
      stream->read(&p.y);
      return;
   }
   if(stream->readFlag())
   {
      U32 maxx = Game::PlayerHorizScopeDistance * 2;
      U32 maxy = Game::PlayerVertScopeDistance * 2;

      F32 dx = F32(stream->readRangedU32(0, maxx)) - Game::PlayerHorizScopeDistance;
      F32 dy = F32(stream->readRangedU32(0, maxy)) - Game::PlayerVertScopeDistance;

      Point delta(dx, dy);
      p = mServerPosition + delta;
   }
   else
   {
      stream->read(&p.x);
      stream->read(&p.y);
   }
}

};