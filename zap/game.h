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

#ifndef _GAME_H_
#define _GAME_H_

#include "point.h"
#include "gameConnection.h"
#include "../tnl/tnlNetObject.h"
#include "gridDB.h"

///
/// Zap - a 2D space game demonstrating the full capabilities of the
/// Torque Network Library.
///
/// The Zap example game is a 2D vector-graphics game that utilizes
/// some of the more advanced features of the TNL.  Zap also demonstrates
/// the use of client-side prediction, and interpolation to present
/// a consistent simulation to clients over a connection with perceptible
/// latency.
///
/// Zap can run in 3 modes - as a client, a client and server, or a dedicated
/// server.  The dedicated server option is available only as a launch
/// parameter from the command line.
///
/// If it is run as a client, Zap uses the GLUT library to perform 
/// cross-platform window intialization, event processing and OpenGL setup.
///
/// Zap implements a simple game framework.  The GameObject class is
/// the root class for all of the various objects in the Zap world, including
/// Ship, Barrier and Projectile instances.  The Game class, which is instanced
/// once for the client and once for the server, manages the current
/// list of GameObject instances.
///
/// Zap clients can connect to servers directly that are on the same LAN
/// or for which the IP address is known.  Zap is also capable of talking
/// to the TNL master server and using its arranged connection functionality
/// to talk to servers.
///
/// The simplified user interface for Zap is managed entirely through
/// subclasses of the UserInterface class.  Each UserInterface subclass
/// represents one "screen" in the UI.  The GameUserInterface is the most complicated,
/// being responsible for the user interface while the client is actually
/// playing a game.  The only other somewhat complicated UI is the 
/// QueryServersUserInterface class, which implements a full server browser
/// for choosing from a list of LAN and master server queried servers.
///
///
namespace Zap
{

class MasterServerConnection;
class GameNetInterface;
class GameType;

/// Base class for server and client Game subclasses.  The Game
/// base class manages all the objects in the game simulation on
/// either the server or the client, and is responsible for
/// managing the passage of time as well as rendering.
class Game
{
protected:
   U32 mLastIdleTime;
   U32 mNextMasterTryTime;
   F32 mGridSize;

   struct DeleteRef
   {
      GameObject *theObject;
      U32 delay;

      DeleteRef(GameObject *o = NULL, U32 d = 0);
   };

   GridDatabase mDatabase;
   Vector<GameObject *> mGameObjects;
   Vector<DeleteRef> mPendingDeleteObjects;

   RefPtr<GameNetInterface> mNetInterface;

   SafePtr<MasterServerConnection> mConnectionToMaster;
   SafePtr<GameType> mGameType;

   enum {
      NumStars = 2048,
      WorldSize = 4096,
      DefaultGridSize = 256,
      MasterServerConnectAttemptDelay = 60000,
   };
   Point mStars[NumStars];

public:
   Game(const Address &theBindAddress);

   void deleteObject(GameObject *theObject, U32 delay);

   void addToGameObjectList(GameObject *theObject);
   void removeFromGameObjectList(GameObject *theObject);

   void setGridSize(F32 gridSize) { mGridSize = gridSize; }

   F32 getGridSize() { return mGridSize; }

   virtual bool isServer() = 0;
   virtual void idle(U32 timeDelta) = 0;

   void checkConnectionToMaster(U32 timeDelta);
   MasterServerConnection *getConnectionToMaster();

   GameNetInterface *getNetInterface();
   GridDatabase *getGridDatabase() { return &mDatabase; }

   GameType *getGameType();
   void setGameType(GameType *theGameType);

   void processDeleteList(U32 timeDelta);
};

class ServerGame : public Game
{
   U32 mPlayerCount;
   U32 mMaxPlayers;
   const char *mHostName;
public:
   U32 getPlayerCount() { return mPlayerCount; }
   U32 getMaxPlayers() { return mMaxPlayers; }
   const char *getHostName() { return mHostName; }

   void addClient(GameConnection *theConnection);
   void removeClient(GameConnection *theConnection);
   ServerGame(const Address &theBindAddress, U32 maxPlayers, const char *hostName);

   void loadLevel(const char *fileName);
   void processLevelLoadLine(int argc, const char **argv);
   bool isServer() { return true; }
   void idle(U32 timeDelta);
};

class ClientGame : public Game
{
   SafePtr<GameConnection> mConnectionToServer; // If this is a client game, this is the connection to the server
public:
   ClientGame(const Address &bindAddress) : Game(bindAddress) {}

   bool hasValidControlObject();
   bool isConnectedToServer();

   void setConnectionToServer(GameConnection *connection);
   GameConnection *getConnectionToServer();
   void render();
   bool isServer() { return false; }
   void idle(U32 timeDelta);
};

extern ServerGame *gServerGame;
extern ClientGame *gClientGame;
extern Address gMasterAddress;

extern void hostGame(bool dedicated, Address bindAddress);
extern void joinGame(Address remoteAddress, bool isFromMaster, bool local = false);
extern void endGame();

};

#endif
