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

#include "gameType.h"
#include "ship.h"
#include "flagItem.h"
#include "glutInclude.h"

namespace Zap
{
class Ship;
class FootballFlag;
class FootballZone;

class FootballGameType : public GameType
{
   typedef GameType Parent;

   Vector<FootballZone*> mZones;
public:
   void shipTouchFlag(Ship *theShip, FlagItem *theFlag);
   void flagDropped(const StringTableEntry &playerName, S32 flagTeamIndex);
   void addZone(FootballZone *z);
   void shipTouchZone(Ship *s, FootballZone *z);

   TNL_DECLARE_CLASS(FootballGameType);
};

TNL_IMPLEMENT_NETOBJECT(FootballGameType);


class FootballZone : public GameObject
{
   typedef GameObject Parent;
   Vector<Point> mPolyBounds;
   enum {
      MaxPoints = 10,

      FlashDelay = 500,
      FlashCount = 5,

      InitialMask = BIT(0),
      TeamMask = BIT(1),
   };
   S32 mFlashCount;
   Timer mFlashTimer;
public:
   FootballZone()
   {
      mTeam = -1;
      mNetFlags.set(Ghostable);
      mObjectTypeMask = CommandMapVisType;
      mFlashCount = 0;
   }

   void render()
   {
      F32 alpha = 0.5;
      Color theColor = getGame()->getGameType()->getTeamColor(getTeam());
      glColor3f(theColor.r * 0.5, theColor.g * 0.5, theColor.b * 0.5);
      glBegin(GL_POLYGON);
      for(S32 i = 0; i < mPolyBounds.size(); i++)
         glVertex2f(mPolyBounds[i].x, mPolyBounds[i].y);
      glEnd();
      if(mFlashCount & 1)
         glColor3f(theColor.r, theColor.g, theColor.b);
      else
         glColor3f(theColor.r * 0.7, theColor.g * 0.7, theColor.b * 0.7);

      glBegin(GL_LINE_LOOP);
      for(S32 i = 0; i < mPolyBounds.size(); i++)
         glVertex2f(mPolyBounds[i].x, mPolyBounds[i].y);
      glEnd();
   }

   S32 getRenderSortValue()
   {
      return -1;
   }

   void processArguments(S32 argc, const char **argv)
   {
      if(argc < 6)
         return;

      for(S32 i = 1; i < argc; i += 2)
      {
         Point p;
         p.x = atof(argv[i-1]) * getGame()->getGridSize();
         p.y = atof(argv[i]) * getGame()->getGridSize();
         mPolyBounds.push_back(p);
      }
      computeExtent();
   }

   void setTeam(S32 team)
   {
      mTeam = team;
      setMaskBits(TeamMask);
   }

   void onAddedToGame(Game *theGame)
   {
      if(!isGhost())
      {
         setScopeAlways();
         ((FootballGameType *) theGame->getGameType())->addZone(this);
      }
   }

   void computeExtent()
   {
      Rect extent(mPolyBounds[0], mPolyBounds[0]);
      for(S32 i = 1; i < mPolyBounds.size(); i++)
         extent.unionPoint(mPolyBounds[i]);
      setExtent(extent);
   }

   bool getCollisionPoly(Vector<Point> &polyPoints)
   {
      for(S32 i = 0; i < mPolyBounds.size(); i++)
         polyPoints.push_back(mPolyBounds[i]);
      return true;
   }

   bool collide(GameObject *hitObject)
   {
      if(!isGhost() && (hitObject->getObjectTypeMask() & ShipType))
         ((FootballGameType *) getGame()->getGameType())->shipTouchZone((Ship *) hitObject, this);

      return false;
   }

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
   {
      if(stream->writeFlag(updateMask & InitialMask))
      {
         stream->writeEnum(mPolyBounds.size(), MaxPoints);
         for(S32 i = 0; i < mPolyBounds.size(); i++)
         {
            stream->write(mPolyBounds[i].x);
            stream->write(mPolyBounds[i].y);
         }
      }
      if(stream->writeFlag(updateMask & TeamMask))
         stream->write(mTeam);
      return 0;
   }

   void unpackUpdate(GhostConnection *connection, BitStream *stream)
   {
      if(stream->readFlag())
      {
         S32 size = stream->readEnum(MaxPoints);
         for(S32 i = 0; i < size; i++)
         {
            Point p;
            stream->read(&p.x);
            stream->read(&p.y);
            mPolyBounds.push_back(p);
         }
         if(size)
            computeExtent();
      }
      if(stream->readFlag())
      {
         stream->read(&mTeam);
         if(!isInitialUpdate())
         {
            mFlashTimer.reset(FlashDelay);
            mFlashCount = FlashCount;
         }
      }
   }

   void idle(GameObject::IdleCallPath path)
   {
      if(path != GameObject::ClientIdleMainRemote || mFlashCount == 0)
         return;

      if(mFlashTimer.update(mCurrentMove.time))
      {
         mFlashTimer.reset(FlashDelay);
         mFlashCount--;
      }
   }

   TNL_DECLARE_CLASS(FootballZone);
};

TNL_IMPLEMENT_NETOBJECT(FootballZone);


void FootballGameType::shipTouchFlag(Ship *theShip, FlagItem *theFlag)
{
   GameConnection *controlConnection = theShip->getControllingClient();
   S32 clientIndex = findClientIndexByConnection(controlConnection);
   if(clientIndex == -1)
      return;

   static StringTableEntry takeString("%e0 of team %e1 has the flag!");
   Vector<StringTableEntry> e;
   ClientRef &cl = mClientList[clientIndex];
   e.push_back(cl.name);
   e.push_back(mTeams[cl.teamId].name);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
   theFlag->mountToShip(theShip);
}

void FootballGameType::flagDropped(const StringTableEntry &playerName, S32 flagTeamIndex)
{
   static StringTableEntry dropString("%e0 dropped the flag!");
   Vector<StringTableEntry> e;
   e.push_back(playerName);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagDrop, dropString, e);
}

void FootballGameType::addZone(FootballZone *z)
{
   mZones.push_back(z);
}

void FootballGameType::shipTouchZone(Ship *s, FootballZone *z)
{
   if(z->getTeam() == s->getTeam())
      return;

   S32 i;
   for(i = 0; i < s->mMountedItems.size(); i++)
      if(s->mMountedItems[i].isValid() && (s->mMountedItems[i]->getObjectTypeMask() & FlagType))
         break;
   if(i == s->mMountedItems.size())
      return;

   S32 oldTeam = z->getTeam();
   if(oldTeam != -1)
   {
      static StringTableEntry takeString("%e0 captured a zone from team %e1!");
      Vector<StringTableEntry> e;
      e.push_back(s->mPlayerName);
      e.push_back(mTeams[oldTeam].name);

      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
      setTeamScore(oldTeam, mTeams[oldTeam].score - 1);
   }
   else
   {
      static StringTableEntry takeString("%e0 captured an unclaimed zone!");
      Vector<StringTableEntry> e;
      e.push_back(s->mPlayerName);
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
   }
   setTeamScore(s->getTeam(), mTeams[s->getTeam()].score + 1);
   z->setTeam(s->getTeam());
   for(S32 i = 0; i < mZones.size(); i++)
   {
      if(mZones[i]->getTeam() != s->getTeam())
         return;
   }
   
   // score another point
   setTeamScore(s->getTeam(), mTeams[s->getTeam()].score + 1);

   static StringTableEntry tdString("Team %e0 scored a touchdown!");
   Vector<StringTableEntry> e;
   e.push_back(mTeams[s->getTeam()].name);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagCapture, tdString, e);

   for(S32 i = 0; i < mZones.size(); i++)
      mZones[i]->setTeam(-1);
   for(S32 i = 0; i < s->mMountedItems.size(); i++)
   {
      Item *theItem = s->mMountedItems[i];
      FlagItem *mountedFlag = dynamic_cast<FlagItem *>(theItem);
      if(mountedFlag)
      {
         mountedFlag->dismount();
         mountedFlag->sendHome();
      }
   }
}

};