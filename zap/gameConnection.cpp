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

#include "gameConnection.h"
#include "gameObject.h"

#include "UIGame.h"
#include "UIMenus.h"

namespace Zap
{

// Global list of clients (if we're a server).
GameConnection GameConnection::gClientList;

TNL_IMPLEMENT_NETCONNECTION(GameConnection, NetClassGroupGame, true);

GameConnection::GameConnection(Game *game)
{
   mNext = mPrev = this;
   mCreateTime = Platform::getRealMilliseconds();
   highSendIndex[0] = 0;
   highSendIndex[1] = 0;
   highSendIndex[2] = 0;
   firstMoveIndex = 1;
   theGame = game;
   setTranslatesStrings();
}

GameConnection::~GameConnection()
{
   // unlink ourselves if we're in the client list
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;

   // Tell the user...
   logprintf("%s disconnected", getNetAddress().toString());
}

void GameConnection::setControlObject(GameObject *theObject)
{
   if(controlObject.isValid())
      controlObject->setControllingClient(NULL);

   controlObject = theObject;
   setScopeObject(theObject);

   if(theObject)
      theObject->setControllingClient(this);
}

void GameConnection::packetReceived(PacketNotify *notify)
{
   for(; firstMoveIndex < ((GamePacketNotify *) notify)->firstUnsentMoveIndex; firstMoveIndex++)
      pendingMoves.erase(U32(0));
   Parent::packetReceived(notify);
}

/// Adds this connection to the doubly linked list of clients.
void GameConnection::linkToClientList()
{
   mNext = gClientList.mNext;
   mPrev = gClientList.mNext->mPrev;
   mNext->mPrev = this;
   mPrev->mNext = this;
}

void GameConnection::writePacket(BitStream *bstream, PacketNotify *notify)
{
   if(isConnectionToServer())
   {
      U32 firstSendIndex = highSendIndex[0];
      if(firstSendIndex < firstMoveIndex)
         firstSendIndex = firstMoveIndex;

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
      highSendIndex[0] = highSendIndex[1];
      highSendIndex[1] = highSendIndex[2];
      highSendIndex[2] = ((GamePacketNotify *) notify)->firstUnsentMoveIndex;
   }
   else
   {
      S32 ghostIndex = -1;
      if(controlObject.isValid())
         ghostIndex = getGhostIndex(controlObject);
      if(bstream->writeFlag(ghostIndex != -1))
      {
         bstream->writeInt(ghostIndex, GhostConnection::GhostIdBitSize);
         controlObject->writeControlState(bstream);
      }
   }
   Parent::writePacket(bstream, notify);
}

void GameConnection::readPacket(BitStream *bstream)
{
   if(isConnectionToClient())
   {
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
            controlObject->processServerMove(&theMove);
         firstMoveIndex++;
      }
   }
   else
   {
      if(bstream->readFlag())
      {
         U32 ghostIndex = bstream->readInt(GhostConnection::GhostIdBitSize);
         controlObject = (GameObject *) resolveGhost(ghostIndex);
         controlObject->readControlState(bstream);
         for(S32 i = 0; i < pendingMoves.size(); i++)
         {
            controlObject->processClientMove(&pendingMoves[i], true);
         }
      }
   }
   Parent::readPacket(bstream);
}

void GameConnection::writeConnectRequest(BitStream *stream)
{
   Parent::writeConnectRequest(stream);

   stream->writeString(playerName.getString());
}

bool GameConnection::readConnectRequest(BitStream *stream, const char **errorString)
{
   if(!Parent::readConnectRequest(stream, errorString))
      return false;
   char buf[256];

   stream->readString(buf);
   playerName = buf;
   return true;
}

U32 GameConnection::currentClientId = 0;

void GameConnection::onConnectionEstablished(bool isInitiator)
{
   Parent::onConnectionEstablished(isInitiator);

   if(isInitiator)
   {
      setGhostFrom(false);
      setGhostTo(true);
      logprintf("%s - connected to server.", getNetAddressString());
      setFixedRateParameters(50, 50, 2000, 2000);
   }
   else
   {
      mClientId = ++currentClientId;

      linkToClientList();
      gServerGame->addClient(this);
      setGhostFrom(true);
      setGhostTo(false);
      activateGhosting();
      logprintf("%s - client connected.", getNetAddressString());
      setFixedRateParameters(50, 50, 2000, 2000);
   }
}

void GameConnection::onConnectionTerminated(const char *reason)
{
   if(isConnectionToServer())
   {
      gMainMenuUserInterface.activate();
   }
   else
   {
      gServerGame->removeClient(this);
   }
}

void GameConnection::onDisconnect(const char *reason)
{
   onConnectionTerminated(avar("Server disconnect - %s", reason));
}

void GameConnection::onTimedOut()
{
   onConnectionTerminated("Connection timed out.");
}

void GameConnection::onConnectTimedOut()
{
   gMainMenuUserInterface.activate();
}

};