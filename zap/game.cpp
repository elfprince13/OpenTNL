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

#include "game.h"
#include "../tnl/tnl.h"
#include "../tnl/tnlRandom.h"
#include "../tnl/tnlGhostConnection.h"
#include "../tnl/tnlNetInterface.h"
#include "gameNetInterface.h"
#include "masterConnection.h"
#include "glutInclude.h"

using namespace TNL;
#include "gameObject.h"
#include "ship.h"
#include "UIGame.h"
#include "SweptEllipsoid.h"
#include "sparkManager.h"
#include "barrier.h"
#include "gameLoader.h"
#include "gameType.h"
#include "sfx.h"

namespace Zap
{

// global Game objects
ServerGame *gServerGame = NULL;
ClientGame *gClientGame = NULL;

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

Game::Game(const Address &theBindAddress)
{
   mNextMasterTryTime = 0;

   mLastIdleTime = Platform::getRealMilliseconds();

   // create random stars
   for(U32 i = 0; i < NumStars; i++)
   {
      mStars[i].x = Random::readF() * WorldSize;
      mStars[i].y = Random::readF() * WorldSize;
   }

   mNetInterface = new GameNetInterface(theBindAddress, this);
}

GameNetInterface *Game::getNetInterface()
{
   return mNetInterface;
}

MasterServerConnection *Game::getConnectionToMaster()
{
   return mConnectionToMaster;
}

GameType *Game::getGameType()
{
   return mGameType;
}

void Game::setGameType(GameType *theGameType)
{
   mGameType = theGameType;
}

void Game::checkConnectionToMaster(U32 timeDelta)
{
   if(!mConnectionToMaster.isValid())
   {
      if(mNextMasterTryTime < timeDelta)
      {
         mConnectionToMaster = new MasterServerConnection(isServer());
         mConnectionToMaster->connect(mNetInterface, gMasterAddress);
         mNextMasterTryTime = MasterServerConnectAttemptDelay;
      }
      else
         mNextMasterTryTime -= timeDelta;
   }
}

Game::DeleteRef::DeleteRef(GameObject *o, U32 d)
{
   theObject = o;
   delay = d;
}

void Game::deleteObject(GameObject *theObject, U32 delay)
{
   mPendingDeleteObjects.push_back(DeleteRef(theObject, delay));
}

void Game::processDeleteList(U32 timeDelta)
{
   for(S32 i = 0; i < mPendingDeleteObjects.size(); )
   {
      if(timeDelta > mPendingDeleteObjects[i].delay)
      {
         GameObject *g = mPendingDeleteObjects[i].theObject;
         delete g;
         mPendingDeleteObjects.erase_fast(i);
      }
      else
      {
         mPendingDeleteObjects[i].delay -= timeDelta;
         i++;
      }
   }
}

void Game::addToGameObjectList(GameObject *theObject)
{
   mGameObjects.push_back(theObject);
}

void Game::removeFromGameObjectList(GameObject *theObject)
{
   for(S32 i = 0; i < mGameObjects.size(); i++)
   {
      if(mGameObjects[i] == theObject)
      {
         mGameObjects.erase_fast(i);
         return;
      }
   }
   TNLAssert(0, "Object not in game's list!");
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

ServerGame::ServerGame(const Address &theBindAddress, U32 maxPlayers, const char *hostName)
 : Game(theBindAddress)
{
   mPlayerCount = 0;
   mMaxPlayers = maxPlayers;
   mHostName = hostName;

   mNetInterface->setAllowsConnections(true);

   // Load a level
   loadLevel("level1.txt");
}

void ServerGame::loadLevel(const char *fileName)
{
   mGridSize = DefaultGridSize;
   GameLoader::initGameFromFile(this, fileName);
}

void ServerGame::processLevelLoadLine(int argc, const char **argv)
{
   if(!stricmp(argv[0], "GridSize"))
   {
      if(argc < 2)
         return;
      mGridSize = atof(argv[1]);
   }
   else if(mGameType.isNull() || !mGameType->processLevelItem(argc, argv))
   {
      TNL::Object *theObject = TNL::Object::create(argv[0]);
      GameObject *object = dynamic_cast<GameObject*>(theObject);
      if(!object)
      {
         logprintf("Invalid object type in level file: %s", argv[0]);
         delete theObject;
      }
      else
      {
         object->addToGame(this);
         object->processArguments(argc - 1, argv + 1);
      }
   }
}

void ServerGame::controlObjectForClientKilled(GameConnection *theConnection)
{
   if(mGameType.isValid())
      mGameType->controlObjectForClientKilled(theConnection);
}

void ServerGame::addClient(GameConnection *theConnection)
{
   if(mGameType.isValid())
      mGameType->serverAddClient(theConnection);
}

void ServerGame::removeClient(GameConnection *theConnection)
{
   if(mGameType.isValid())
      mGameType->serverRemoveClient(theConnection);
}

void ServerGame::idle(U32 timeDelta)
{
   mNetInterface->checkIncomingPackets();
   Game::checkConnectionToMaster(timeDelta);
   for(S32 i = 0; i < mGameObjects.size(); i++)
   {
      mGameObjects[i]->processServer(timeDelta);
   }
   processDeleteList(timeDelta);
   mNetInterface->processConnections();
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------


bool ClientGame::hasValidControlObject()
{
   return mConnectionToServer.isValid() && mConnectionToServer->getControlObject();
}

bool ClientGame::isConnectedToServer()
{
   return mConnectionToServer.isValid() && mConnectionToServer->getConnectionState() == NetConnection::Connected;
}

GameConnection *ClientGame::getConnectionToServer()
{
   return mConnectionToServer;
}

void ClientGame::setConnectionToServer(GameConnection *theConnection)
{
   TNLAssert(mConnectionToServer.isNull(), "Error, a connection already exists here.");
   mConnectionToServer = theConnection;
}

void ClientGame::idle(U32 timeDelta)
{
   mNetInterface->checkIncomingPackets();

   Game::checkConnectionToMaster(timeDelta);
   // only update at most MaxMoveTime milliseconds;
   if(timeDelta > Move::MaxMoveTime)
      timeDelta = Move::MaxMoveTime;

   Move *theMove = gGameUserInterface.getCurrentMove();
   theMove->prepare();

   theMove->time = timeDelta;

   if(mConnectionToServer.isValid())
   {
      mConnectionToServer->addPendingMove(theMove);

      for(S32 i = 0; i < mGameObjects.size(); i++)
      {
         if(mGameObjects[i] == mConnectionToServer->getControlObject())
            mGameObjects[i]->processClientMove(theMove, false);
         else
            mGameObjects[i]->processClient(timeDelta);
      }
   }
   processDeleteList(timeDelta);
   SparkManager::tick((F32)timeDelta / 1000.f);
   SFXObject::process();

   mNetInterface->processConnections();
}

void ClientGame::render()
{
   if(!hasValidControlObject())
      return;

   GameObject *u = mConnectionToServer->getControlObject();
   Point position = u->getRenderPos();

   glPushMatrix();

   glTranslatef(-position.x, -position.y, 0);

   // render the stars
   glPointSize( 1.0f );
   glColor3f(0.8, 0.8, 1.0);
   glBegin(GL_POINTS);
   for(U32 i = 0; i < NumStars; i++)
      glVertex2f(mStars[i].x, mStars[i].y);
   glEnd();

   // render the objects
   Vector<GameObject *> renderObjects;
   
   Point screenSize(400, 300);
   Rect extentRect(position - screenSize, position + screenSize);
   mDatabase.findObjects(AllObjectTypes, renderObjects, extentRect);

   for(S32 i = 0; i < renderObjects.size(); i++)
      renderObjects[i]->render();

   SparkManager::render();

   glPopMatrix();
}

};