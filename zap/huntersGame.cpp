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

#include "huntersGame.h"
#include "glutInclude.h"
#include "UIGame.h"
#include "sfx.h"
#include "gameNetInterface.h"
#include "ship.h"

namespace Zap
{

void renderHunterFlag(Point pos, Color c)
{
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);

   glColor3f(c.r, c.g, c.b);
   glBegin(GL_LINES);
   glVertex2f(-15, -15);
   glVertex2f(15, -5);

   glVertex2f(15, -5);
   glVertex2f(-15, 5);

   glVertex2f(-15, -10);
   glVertex2f(10, -5);

   glVertex2f(10, -5);
   glVertex2f(-15, 0);
   glColor3f(1,1,1);
   glVertex2f(-15, -15);
   glVertex2f(-15, 15);
   glEnd();

   glPopMatrix();
}

TNL_IMPLEMENT_NETOBJECT(HuntersGameType);

TNL_IMPLEMENT_NETOBJECT_RPC(HuntersGameType, s2cSetNexusTimer, (U32 nexusTime, bool canCap),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   mNexusReturnTimer.reset(nexusTime);
   mCanNexusCap = canCap;
}

TNL_IMPLEMENT_NETOBJECT_RPC(HuntersGameType, s2cHuntersMessage, (U32 msgIndex, StringTableEntryRef clientName, U32 flagCount),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   if(msgIndex == HuntersMsgScore)
   {
      SFXObject::play(SFXFlagCapture);
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                  "%s returned %d flag(s) to the Nexus!", 
                  clientName.getString(),
                  flagCount);
   }
   else if(msgIndex == HuntersMsgYardSale)
   {
      SFXObject::play(SFXFlagSnatch);
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                  "%s is having a YARD SALE!", 
                  clientName.getString());
   }
   else if(msgIndex == HuntersMsgGameOverWin)
   {
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "Player %s wins the game!", 
                     clientName.getString());
      SFXObject::play(SFXFlagCapture);
   }
   else if(msgIndex == HuntersMsgGameOverTie)
   {
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "The game ended in a tie.");
      SFXObject::play(SFXFlagDrop);
   }
}

HuntersGameType::HuntersGameType() : GameType()
{
   mCanNexusCap = false;
   mNexusReturnDelay = 60 * 1000;
   mNexusCapDelay = 15 * 1000;
   mNexusReturnTimer.reset(mNexusReturnDelay);
   mNexusCapTimer.reset(0);
}

void HuntersGameType::processArguments(S32 argc, const char **argv)
{
   if(argc > 0)
   {
      mGameTimer.reset(U32(atof(argv[0]) * 60 * 1000));
      if(argc > 1)
      {
         mNexusReturnDelay = atoi(argv[1]) * 60 * 1000;
         if(argc > 2)
         {
            mNexusCapDelay = atoi(argv[2]) * 1000;
            if(argc > 3)
               mTeamScoreLimit = atoi(argv[3]);
         }
      }
   }
   mNexusReturnTimer.reset(mNexusReturnDelay);
}

void HuntersGameType::shipTouchNexus(Ship *theShip, HuntersNexusObject *theNexus)
{
   HuntersFlagItem *theFlag = NULL;
   for(S32 i = theShip->mMountedItems.size() - 1; i >= 0; i--)
   {
      Item *theItem = theShip->mMountedItems[i];
      theFlag = dynamic_cast<HuntersFlagItem *>(theItem);
      if(theFlag)
         break;
   }

   U32 score = 0;
   for(U32 count = 1; count < theFlag->getFlagCount() + 1; count++)
      score += (count * 10);

   S32 cIndex = findClientIndexByConnection(theShip->getControllingClient());
   mClientList[cIndex].score += score;

   if(theFlag->getFlagCount() > 0)
      s2cHuntersMessage(HuntersMsgScore, theShip->mPlayerName.getString(), theFlag->getFlagCount());
   theFlag->changeFlagCount(0);
}

void HuntersGameType::onGhostAvailable(GhostConnection *theConnection)
{
   Parent::onGhostAvailable(theConnection);

   NetObject::setRPCDestConnection(theConnection);
   if(mCanNexusCap)
      s2cSetNexusTimer(mNexusCapTimer.getCurrent(), mCanNexusCap);
   else
      s2cSetNexusTimer(mNexusReturnTimer.getCurrent(), mCanNexusCap);
   NetObject::setRPCDestConnection(NULL);
}

void HuntersGameType::idle(GameObject::IdleCallPath path)
{
   Parent::idle(path);

   U32 deltaT = mCurrentMove.time;
   if(isGhost())
   {
      mNexusReturnTimer.update(deltaT);
      return;
   }

   if(mNexusReturnTimer.update(deltaT))
   {
      mNexusCapTimer.reset(mNexusCapDelay);
      mCanNexusCap = true;
      s2cSetNexusTimer(mNexusCapTimer.getCurrent(), mCanNexusCap);
   }
   else if(mNexusCapTimer.update(deltaT))
   {
      mNexusReturnTimer.reset(mNexusReturnDelay);
      mCanNexusCap = false;
      s2cSetNexusTimer(mNexusReturnTimer.getCurrent(), mCanNexusCap);
   }
}

void HuntersGameType::renderInterfaceOverlay(bool scoreboardVisible)
{
   Parent::renderInterfaceOverlay(scoreboardVisible);

   glColor3f(1,1,1);
   U32 timeLeft = mNexusReturnTimer.getCurrent();
   U32 minsRemaining = timeLeft / (60000);
   U32 secsRemaining = (timeLeft - (minsRemaining * 60000)) / 1000;
   UserInterface::drawStringf(720, 550, 20, "%02d:%02d", minsRemaining, secsRemaining);
}

void HuntersGameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   Parent::controlObjectForClientKilled(theClient, clientObject, killerObject);

   Ship *theShip = dynamic_cast<Ship *>(clientObject);
   if(!theShip)
      return;

   // check for yard sale
   for(S32 i = theShip->mMountedItems.size() - 1; i >= 0; i--)
   {
      Item *theItem = theShip->mMountedItems[i];
      HuntersFlagItem *theFlag = dynamic_cast<HuntersFlagItem *>(theItem);
      if(theFlag)
      {
         if(theFlag->getFlagCount() > 2)
            s2cHuntersMessage(HuntersMsgYardSale, theShip->mPlayerName.getString(), 0);

         return;
      }
   }
}

void HuntersGameType::spawnShip(GameConnection *theClient)
{
   Parent::spawnShip(theClient);

   HuntersFlagItem *newFlag = new HuntersFlagItem(theClient->getControlObject()->getActualPos());
   newFlag->addToGame(getGame());
   newFlag->mountToShip((Ship *) theClient->getControlObject());
   newFlag->changeFlagCount(0);
}

void HuntersGameType::gameOverManGameOver()
{
   Parent::gameOverManGameOver();
   
   bool tied = false;
   ClientRef winningClient = mClientList[0];
   for(S32 i = 1; i < mClientList.size(); i++)
   {
      if(mClientList[i].score == winningClient.score)
         tied = true;
      else if(mClientList[i].score > winningClient.score)
      {
         winningClient = mClientList[i];
         tied = false;
      }
   }
   if(tied)
      s2cHuntersMessage(HuntersMsgGameOverTie, StringTableEntry(), 0);
   else
      s2cHuntersMessage(HuntersMsgGameOverWin, winningClient.name.getString(), 0);
}

TNL_IMPLEMENT_NETOBJECT(HuntersFlagItem);

HuntersFlagItem::HuntersFlagItem(Point pos) : Item(pos, true, 30, 4)
{
   mObjectTypeMask |= CommandMapVisType;
   mNetFlags.set(Ghostable);
   mFlagCount = 0;
}

void HuntersFlagItem::renderItem(Point pos)
{
   Point offset = pos;

   if(mIsMounted)
      offset.set(pos.x + 15, pos.y - 15);

   Color c;
   GameType *gt = getGame()->getGameType();

   c = gt->mTeams[0].color;

   renderHunterFlag(offset, c);

   if(mIsMounted)
      UserInterface::drawStringf(offset.x - 5, offset.y - 30, 12, "%d", mFlagCount);
}

void HuntersFlagItem::onMountDestroyed()
{
   if(!mMount.isValid())
      return;

   // drop at least one flag plus as many as the ship
   //  carries
   for(U32 i = 0; i < mFlagCount + 1; i++)
   {
      HuntersFlagItem *newFlag = new HuntersFlagItem(mMount->getActualPos());
      newFlag->addToGame(getGame());

      F32 th = Random::readF() * 2 * 3.14;
      F32 f = (Random::readF() * 2 - 1) * 100;
      Point vel(cos(th) * f, sin(th) * f);
      vel += mMount->getActualVel();
      
      newFlag->setActualVel(vel);
   }
   changeFlagCount(0);
   
   // now delete yourself
   removeFromDatabase();
   deleteObject();
}

void HuntersFlagItem::setActualVel(Point v)
{
   mMoveState[ActualState].vel = v;
   setMaskBits(WarpPositionMask | PositionMask);
}

bool HuntersFlagItem::collide(GameObject *hitObject)
{
   if(isGhost() || mIsMounted || !mIsCollideable)
      return false;

   if(hitObject->getObjectTypeMask() & BarrierType)
      return true;

   if(!(hitObject->getObjectTypeMask() & ShipType))
      return false;

   Ship *theShip = static_cast<Ship *>(hitObject);
   if(!theShip)
      return false;

   if(theShip->hasExploded)
      return false;

   // don't mount to ship, instead increase current mounted HuntersFlag
   //  flagCount, and remove collided flag from game
   for(S32 i = theShip->mMountedItems.size() - 1; i >= 0; i--)
   {
      Item *theItem = theShip->mMountedItems[i];
      HuntersFlagItem *theFlag = dynamic_cast<HuntersFlagItem *>(theItem);
      if(theFlag)
      {
         theFlag->changeFlagCount(theFlag->getFlagCount() + 1);
         break;
      }
   }

   mIsCollideable = false;
   removeFromDatabase();
   deleteObject();
   return true;
}

U32 HuntersFlagItem::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(connection, updateMask, stream);
   if(stream->writeFlag(updateMask & FlagCountMask))
      stream->write(mFlagCount);

   return retMask;
}

void HuntersFlagItem::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection, stream);

   if(stream->readFlag())
      stream->read(&mFlagCount);
}

TNL_IMPLEMENT_NETOBJECT(HuntersNexusObject);

HuntersNexusObject::HuntersNexusObject()
{
   mObjectTypeMask |= CommandMapVisType;
   mNetFlags.set(Ghostable);
   nexusBounds.set(Point(), Point());
}

void HuntersNexusObject::processArguments(S32 argc, const char **argv)
{
   if(argc < 2)
      return;

   Point pos;
   pos.read(argv);
   pos *= getGame()->getGridSize();

   Point ext(50, 50);
   if(argc > 3)
      ext.set(atoi(argv[2]), atoi(argv[3]));

   Point min(pos.x - ext.x, pos.y - ext.y);
   Point max(pos.x + ext.x, pos.y + ext.y);
   nexusBounds.set(min, max);
   setExtent(nexusBounds);
}

void HuntersNexusObject::onAddedToGame(Game *theGame)
{
   if(!isGhost())
   {
      setInterface(theGame->getNetInterface());
      setScopeAlways();
   }
}

void HuntersNexusObject::render()
{
   F32 alpha = 0.2;
   HuntersGameType *theGameType = dynamic_cast<HuntersGameType *>(getGame()->getGameType());
   if(theGameType && theGameType->mCanNexusCap)
      alpha = 0.5;

   Color theColor = getGame()->getGameType()->mTeams[0].color;
   glColor3f(theColor.r * alpha, theColor.g * alpha, theColor.b * alpha);
   glBegin(GL_POLYGON);
      glVertex2f(nexusBounds.min.x, nexusBounds.min.y);
      glVertex2f(nexusBounds.min.x, nexusBounds.max.y);
      glVertex2f(nexusBounds.max.x, nexusBounds.max.y);
      glVertex2f(nexusBounds.max.x, nexusBounds.min.y);
   glEnd();
}

bool HuntersNexusObject::getCollisionPoly(Vector<Point> &polyPoints)
{
   polyPoints.push_back(nexusBounds.min);
   polyPoints.push_back(nexusBounds.max);
   return true;
}

bool HuntersNexusObject::collide(GameObject *hitObject)
{
   if(isGhost())
      return false;

   if(!(hitObject->getObjectTypeMask() & ShipType))
      return false;

   if(((Ship *) hitObject)->hasExploded)
      return false;

   Ship *theShip = (Ship *)hitObject;

   HuntersGameType *theGameType = dynamic_cast<HuntersGameType *>(getGame()->getGameType());
   if(theGameType && theGameType->mCanNexusCap)
      theGameType->shipTouchNexus(theShip, this);

   return false;
}

U32 HuntersNexusObject::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   stream->write(nexusBounds.min.x);
   stream->write(nexusBounds.min.y);
   stream->write(nexusBounds.max.x);
   stream->write(nexusBounds.max.y);
   return 0;
}

void HuntersNexusObject::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   stream->read(&nexusBounds.min.x);
   stream->read(&nexusBounds.min.y);
   stream->read(&nexusBounds.max.x);
   stream->read(&nexusBounds.max.y);
   setExtent(nexusBounds);
}

};
