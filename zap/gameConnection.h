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

#ifndef _GAMECONNECTION_H_
#define _GAMECONNECTION_H_

#include "../tnl/tnl.h"
#include "../tnl/tnlGhostConnection.h"

using namespace TNL;
// some angle conversion functions:
namespace Zap
{

static char *gConnectStatesTable[] = {
      //NotConnected,              ///< Initial state of a NetConnection instance - not connected.
      "Not connected...",
      //AwaitingChallengeResponse, ///< We've sent a challenge request, awaiting the response.
      "Sending challenge request.",
      //SendingPunchPackets,       ///< The state of a pending arranged connection when both sides haven't heard from the other yet
      "Punching through firewalls.",
      //ComputingPuzzleSolution,   ///< We've received a challenge response, and are in the process of computing a solution to its puzzle.
      "Computing puzzle solution.",
      //AwaitingConnectResponse,   ///< We've received a challenge response and sent a connect request.
      "Sent connect request.",
      //ConnectTimedOut,           ///< The connection timed out during the connection process.
      "Connection timed out.",
      //ConnectRejected,           ///< The connection was rejected.
      "Connection rejected.",
      //Connected,                 ///< We've accepted a connect request, or we've received a connect response accept.
      "Connected.",
      //Disconnected,              ///< The connection has been disconnected.
      "Disconnected.",
      //TimedOut,                  ///< The connection timed out.
      "Connection timed out.",
      ""
};

const F32 constantPi = 3.141592f;
const F32 radiansToDegreesConversion = 360.0f / (2 * constantPi);
const F32 radiansToUnitConversion = 1 / (2 * constantPi);
const F32 unitToRadiansConversion = 2 * constantPi;

inline F32 radiansToDegrees(F32 angle)
{
   return angle * radiansToDegreesConversion;
}

inline F32 radiansToUnit(F32 angle)
{
   return angle * radiansToUnitConversion;
}

inline F32 unitToRadians(F32 angle)
{
   return angle * unitToRadiansConversion;
}

struct Move
{
   float left;
   float right;
   float up;
   float down;
   float angle;
   bool fire;
   U32 time;

   enum {
      MaxMoveTime = 127,
   };

   Move() { left = right = up = down = angle = 0; fire = false; time = 32; }

   bool isEqualMove(Move *prev)
   {
      return   prev->left == left &&
               prev->right == right &&
               prev->up == up &&
               prev->down == down &&
               prev->angle == angle &&
               prev->fire == fire;
   }

   void pack(BitStream *stream, Move *prev)
   {
      if(!stream->writeFlag(prev && isEqualMove(prev)))
      {
         stream->writeFloat(left, 4);
         stream->writeFloat(right, 4);
         stream->writeFloat(up, 4);
         stream->writeFloat(down, 4);
         U32 writeAngle = U32(radiansToUnit(angle) * 0xFFF);

         stream->writeInt(writeAngle, 12);
         stream->writeFlag(fire);
      }
      stream->writeRangedU32(time, 0, MaxMoveTime);
   }
   void unpack(BitStream *stream)
   {
      if(!stream->readFlag())
      {
         left = stream->readFloat(4);
         right = stream->readFloat(4);
         up = stream->readFloat(4);
         down = stream->readFloat(4);
         angle = unitToRadians(stream->readInt(12) / F32(0xFFF));
         fire = stream->readFlag();
      }
      time = stream->readRangedU32(0, MaxMoveTime);
   }
   void prepare()
   {
      PacketStream stream;
      pack(&stream, NULL);
      stream.setBytePosition(0);
      unpack(&stream);
   }
};

class GameObject;
class Game;

class GameConnection : public GhostConnection
{
   typedef GhostConnection Parent;
public:
   // move management
   enum {
      MaxPendingMoves = 31,
   };
   Vector<Move> pendingMoves;
   SafePtr<GameObject> controlObject;
   Game *theGame;

   StringTableEntry playerName;
   static U32 currentClientId;
   U32 mClientId;
   U32 mLastClientControlCRC;

   U32 firstMoveIndex;
   U32 highSendIndex[3];

   // The server maintains a linked list of clients...
   GameConnection *mNext;
   GameConnection *mPrev;
   static GameConnection gClientList;

   // Time in milliseconds at which we were created.
   U32 mCreateTime;

   GameConnection(Game *game = NULL);
   ~GameConnection();

   void setPlayerName(const char *string) { playerName = string; }
   void setControlObject(GameObject *theObject);

   GameObject *getControlObject() { return controlObject; }
   U32 getControlCRC();

   void linkToClientList();

   void addPendingMove(Move *theMove)
   {
      if(pendingMoves.size() < MaxPendingMoves)
         pendingMoves.push_back(*theMove);
   }

   struct GamePacketNotify : public GhostConnection::GhostPacketNotify
   {
      U32 firstUnsentMoveIndex;
      GamePacketNotify() { firstUnsentMoveIndex =  0; }
   };
   PacketNotify *allocNotify() { return new GamePacketNotify; }

   void writePacket(BitStream *bstream, PacketNotify *notify);
   void readPacket(BitStream *bstream);

   void packetReceived(PacketNotify *notify);
   void processMoveServer(Move *theMove);

   void writeConnectRequest(BitStream *stream);
   bool readConnectRequest(BitStream *stream, const char **errorString);

   /// Adds this connection to the doubly linked list of clients.
   void onConnectionEstablished(bool isInitiator);

   void onDisconnect(const char *reason);
   void onTimedOut();
   void onConnectionTerminated(const char *reason);

   void onConnectTimedOut();

   TNL_DECLARE_NETCONNECTION(GameConnection);
};

};

#endif
