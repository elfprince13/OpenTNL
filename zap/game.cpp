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
#include "UIMenus.h"
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
      if(gMasterAddress == Address())
         return;
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

Rect Game::computeWorldObjectExtents()
{
   if(!mGameObjects.size())
      return Rect();

   Rect theRect = mGameObjects[0]->getExtent();
   for(S32 i = 0; i < mGameObjects.size(); i++)
      theRect.unionRect(mGameObjects[i]->getExtent());
   return theRect;
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
}

void ServerGame::setLevelList(const char *levelList)
{
   for(;;)
   {
      const char *firstSpace = strchr(levelList, ' ');
      StringTableEntry st;
      if(firstSpace)
         st.setn(levelList, firstSpace - levelList);
      else
         st.set(levelList);

      if(st.getString()[0])
         mLevelList.push_back(st);
      if(!firstSpace)
         break;
      levelList = firstSpace + 1;
   }
   mCurrentLevelIndex = mLevelList.size() - 1;
   cycleLevel();
}

void ServerGame::cycleLevel()
{
   // delete any objects on the delete list:
   processDeleteList(0xFFFFFFFF);

   // delete any objects that may exist
   while(mGameObjects.size())
      delete mGameObjects[0];

   for(GameConnection *walk = GameConnection::getClientList(); walk ; walk = walk->getNextClient())
      walk->resetGhosting();

   mCurrentLevelIndex++;
   if(S32(mCurrentLevelIndex) >= mLevelList.size())
      mCurrentLevelIndex = 0;
   loadLevel(mLevelList[mCurrentLevelIndex].getString());
   if(!getGameType())
   {
      GameType *g = new GameType;
      g->addToGame(this);
   }
   Vector<GameConnection *> connectionList;

   for(GameConnection *walk = GameConnection::getClientList(); walk ; walk = walk->getNextClient())
      connectionList.push_back(walk);

   // now add the connections to the game type, in a random order
   while(connectionList.size())
   {
      U32 index = Random::readI() % connectionList.size();
      GameConnection *gc = connectionList[index];
      connectionList.erase(index);

      if(mGameType.isValid())
         mGameType->serverAddClient(gc);
      gc->activateGhosting();
   }
}

void ServerGame::loadLevel(const char *fileName)
{
   mGridSize = DefaultGridSize;
   initLevelFromFile(fileName);
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

void ServerGame::addClient(GameConnection *theConnection)
{
   if(mGameType.isValid())
      mGameType->serverAddClient(theConnection);
   mPlayerCount++;
}

void ServerGame::removeClient(GameConnection *theConnection)
{
   if(mGameType.isValid())
      mGameType->serverRemoveClient(theConnection);
   mPlayerCount--;
}

void ServerGame::idle(U32 timeDelta)
{
   mNetInterface->checkIncomingPackets();
   Game::checkConnectionToMaster(timeDelta);
   for(S32 i = 0; i < mGameObjects.size(); i++)
   {
      Move thisMove = mGameObjects[i]->getCurrentMove();
      thisMove.time = timeDelta;
      mGameObjects[i]->setCurrentMove(thisMove);
      mGameObjects[i]->idle(GameObject::ServerIdleMainLoop);
   }

   processDeleteList(timeDelta);
   mNetInterface->processConnections();

   if(mLevelSwitchTimer.update(timeDelta))
      cycleLevel();
}

void ServerGame::gameEnded()
{
   mLevelSwitchTimer.reset(LevelSwitchTime);
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

ClientGame::ClientGame(const Address &bindAddress)
 : Game(bindAddress)
{
   mInCommanderMap = false;
   mCommanderZoomDelta = 0;
   // create random stars
   for(U32 i = 0; i < NumStars; i++)
   {
      mStars[i].x = Random::readF();
      mStars[i].y = Random::readF();
   }
}

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

extern void JoystickUpdateMove( Move *theMove );

void ClientGame::idle(U32 timeDelta)
{
   mNetInterface->checkIncomingPackets();

   Game::checkConnectionToMaster(timeDelta);

   // only update at most MaxMoveTime milliseconds;
   if(timeDelta > Move::MaxMoveTime)
      timeDelta = Move::MaxMoveTime;

   if(!mInCommanderMap && mCommanderZoomDelta != 0)
   {
      if(timeDelta > mCommanderZoomDelta)
         mCommanderZoomDelta = 0;
      else
         mCommanderZoomDelta -= timeDelta;
   }
   else if(mInCommanderMap && mCommanderZoomDelta != CommanderMapZoomTime)
   {
      mCommanderZoomDelta += timeDelta;
      if(mCommanderZoomDelta > CommanderMapZoomTime)
         mCommanderZoomDelta = CommanderMapZoomTime;
   }

   Move *theMove = gGameUserInterface.getCurrentMove();

#ifdef TNL_OS_WIN32
   if(OptionsMenuUserInterface::joystickEnabled)
      JoystickUpdateMove(theMove);
#endif
   theMove->time = timeDelta;

   if(mConnectionToServer.isValid())
   {
      mConnectionToServer->addPendingMove(theMove);
      GameObject *controlObject = mConnectionToServer->getControlObject();

      theMove->prepare();

      for(S32 i = 0; i < mGameObjects.size(); i++)
      {
         if(mGameObjects[i] == controlObject)
         {
            mGameObjects[i]->setCurrentMove(*theMove);
            mGameObjects[i]->idle(GameObject::ClientIdleControlMain);
         }
         else
         {
            Move m = mGameObjects[i]->getCurrentMove();
            m.time = timeDelta;
            mGameObjects[i]->setCurrentMove(m);
            mGameObjects[i]->idle(GameObject::ClientIdleMainRemote);
         }
      }

      if(controlObject)
         SFXObject::setListenerParams(controlObject->getRenderPos(),controlObject->getRenderVel());   
   }
   processDeleteList(timeDelta);
   SparkManager::tick((F32)timeDelta * 0.001f);
   SFXObject::process();

   mNetInterface->processConnections();
}

void ClientGame::zoomCommanderMap()
{
   mInCommanderMap = !mInCommanderMap;
   if(mInCommanderMap)
      SFXObject::play(SFXUICommUp);
   else
      SFXObject::play(SFXUICommDown);


   GameConnection *conn = getConnectionToServer();
   
   if(conn)
   {
      if(mInCommanderMap)
         conn->c2sRequestCommanderMap();
      else
         conn->c2sReleaseCommanderMap();
   }
}

void ClientGame::drawStars(F32 alphaFrac, Point cameraPos, Point visibleExtent)
{
   F32 starChunkSize = 1024;

   Point upperLeft = cameraPos - visibleExtent * 0.5f;
   Point lowerRight = cameraPos + visibleExtent * 0.5f;

   upperLeft *= 1 / starChunkSize;
   lowerRight *= 1 / starChunkSize;

   upperLeft.x = floor(upperLeft.x);
   upperLeft.y = floor(upperLeft.y);

   lowerRight.x = floor(lowerRight.x) + 0.5;
   lowerRight.y = floor(lowerRight.y) + 0.5;

   // render the stars
   glPointSize( 1.0f );
   glColor3f(0.8 * alphaFrac, 0.8 * alphaFrac, 1.0 * alphaFrac);

   glPointSize(1);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(2, GL_FLOAT, sizeof(Point), &mStars[0]);

   for(F32 xPage = upperLeft.x; xPage < lowerRight.x; xPage++)
   {
      for(F32 yPage = upperLeft.y; yPage < lowerRight.y; yPage++)
      {
         glPushMatrix();
         glScalef(starChunkSize, starChunkSize, 1);
         glTranslatef(xPage, yPage, 0);
         //glTranslatef(-xPage * starChunkSize, -yPage * starChunkSize, 0);
         glDrawArrays(GL_POINTS, 0, NumStars);
         glPopMatrix();
      }
   }

   glDisableClientState(GL_VERTEX_ARRAY);
}

S32 QSORT_CALLBACK renderSortCompare(GameObject **a, GameObject **b)
{
   return (*a)->getRenderSortValue() - (*b)->getRenderSortValue();
}

void ClientGame::renderCommander()
{
   GameObject *u = mConnectionToServer->getControlObject();
   Point position = u->getRenderPos();

   F32 zoomFrac = mCommanderZoomDelta / F32(CommanderMapZoomTime);
   // Set up the view to show the whole level.
   Rect worldBounds = computeWorldObjectExtents();

   Point worldCenter = worldBounds.getCenter();
   Point worldExtents = worldBounds.getExtents();

   F32 aspectRatio = worldExtents.x / worldExtents.y;
   F32 screenAspectRatio = UserInterface::canvasWidth / F32(UserInterface::canvasHeight);
   if(aspectRatio > screenAspectRatio)
      worldExtents.y *= aspectRatio / screenAspectRatio;
   else
      worldExtents.x *= screenAspectRatio / aspectRatio;

   Point offset = (worldCenter - position) * zoomFrac + position;
   Point visSize(2 * PlayerHorizVisDistance, 2 * PlayerVertVisDistance);
   Point modVisSize = (worldExtents - visSize) * zoomFrac + visSize;

   Point visScale(UserInterface::canvasWidth / modVisSize.x,
                  UserInterface::canvasHeight / modVisSize.y );

   glPushMatrix();
   glTranslatef(400, 300, 0);

   glScalef(visScale.x, visScale.y, 1);
   glTranslatef(-offset.x, -offset.y, 0);

   if(zoomFrac < 0.95)
      drawStars(1 - zoomFrac, offset, modVisSize);

   // render the objects
   Vector<GameObject *> renderObjects;
   mDatabase.findObjects( CommandMapVisType, renderObjects, worldBounds);

   // Deal with rendering sensor volumes

   // get info about the current player
   GameType *gt = gClientGame->getGameType();
   GameObject *co = gClientGame->getConnectionToServer()->getControlObject();

   if(gt && dynamic_cast<Ship*>(co))
   {
      S32 playerId = gt->findClientIndexByName(((Ship *) co)->mPlayerName);
      S32 playerTeam = gt->mClientList[playerId].teamId;
      Color teamColor = gt->mTeams[playerTeam].color;
      
      F32 colorFactor = zoomFrac * 0.35;

      glColor3f(teamColor.r * colorFactor, teamColor.g * colorFactor, teamColor.b * colorFactor);

      for(S32 i = 0; i < renderObjects.size(); i++)
      {
         // If it's a ship, add it to the list.

         if(renderObjects[i]->getObjectTypeMask() & ShipType)
         {
            Ship * so = (Ship*)(renderObjects[i]);
            // Get team of this object.
            S32 ourClientId = gClientGame->getGameType()->findClientIndexByName(so->mPlayerName);
            S32 ourTeam     = gClientGame->getGameType()->mClientList[ourClientId].teamId;

            if(ourTeam == playerTeam)
            {
               Point p = so->getRenderPos();
               
               glBegin(GL_POLYGON);
               glVertex2f(p.x - PlayerHorizScopeDistance, p.y - PlayerVertScopeDistance);
               glVertex2f(p.x + PlayerHorizScopeDistance, p.y - PlayerVertScopeDistance);
               glVertex2f(p.x + PlayerHorizScopeDistance, p.y + PlayerVertScopeDistance);
               glVertex2f(p.x - PlayerHorizScopeDistance, p.y + PlayerVertScopeDistance);
               glEnd();

            }
         }
      }
   }

   renderObjects.sort(renderSortCompare);

   for(S32 i = 0; i < renderObjects.size(); i++)
      renderObjects[i]->render(0);
   for(S32 i = 0; i < renderObjects.size(); i++)
      renderObjects[i]->render(1);

   glPopMatrix();
}

void ClientGame::renderNormal()
{
   GameObject *u = mConnectionToServer->getControlObject();
   Point position = u->getRenderPos();

   glPushMatrix();
   glTranslatef(400, 300, 0);

   glScalef(400 / F32(PlayerHorizVisDistance), 300 / F32(PlayerVertVisDistance), 1);

   glTranslatef(-position.x, -position.y, 0);

   drawStars(1.0, position, Point(PlayerHorizVisDistance * 2, PlayerVertVisDistance * 2));

   // render the objects
   Vector<GameObject *> renderObjects;

   Point screenSize(PlayerHorizVisDistance, PlayerVertVisDistance);
   Rect extentRect(position - screenSize, position + screenSize);
   mDatabase.findObjects(AllObjectTypes, renderObjects, extentRect);

   renderObjects.sort(renderSortCompare);

   for(S32 i = 0; i < renderObjects.size(); i++)
      renderObjects[i]->render(0);
   for(S32 i = 0; i < renderObjects.size(); i++)
      renderObjects[i]->render(1);

   SparkManager::render();
   fxTrail::renderTrails();

   glPopMatrix();

   // Render current ship's energy, too.
   if(mConnectionToServer.isValid())
   {
      Ship *s = dynamic_cast<Ship*>(mConnectionToServer->getControlObject());
      if(s)
      {
         //static F32 curEnergy = 0.f;

         //curEnergy = (curEnergy + s->mEnergy) * 0.5f;
         F32 energy = s->mEnergy / F32(Ship::EnergyMax);
         U32 totalLineCount = 100;

         glColor3f(1, 1 ,1);
         glBegin(GL_LINES);
         glVertex2f(15, 565);
         glVertex2f(15, 585);
         glVertex2f(15 + totalLineCount * 2, 565);
         glVertex2f(15 + totalLineCount * 2, 585);

         F32 halfway = totalLineCount * 0.5;
         F32 full = energy * totalLineCount;
         for(U32 i = 1; i < full; i++)
         {
            /*if(i < halfway)
               glColor3f(1 - i / halfway, 0, i / halfway);
            else
               glColor3f(0, (i - halfway) / halfway, 1);
            */
            glColor3f(0, i / full, 1);
            glVertex2f(15 + i * 2, 565);
            glVertex2f(15 + i * 2, 585);
         }
         glEnd();

         // Show safety line.
         glColor3f(1, 1, 0);
         glBegin(GL_LINES);
         F32 cutoffx = (Ship::EnergyCooldownThreshold * totalLineCount * 2) / Ship::EnergyMax;
         glVertex2f(15 + cutoffx, 562);
         glVertex2f(15 + cutoffx, 589);
         glEnd();

      }
   }
}

void ClientGame::render()
{
   if(!hasValidControlObject())
      return;

   if(mCommanderZoomDelta)
      renderCommander();
   else
      renderNormal();
}

};

