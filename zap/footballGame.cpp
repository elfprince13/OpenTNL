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

#include "goalZone.h"
#include "gameType.h"
#include "ship.h"
#include "flagItem.h"
#include "gameObjectRender.h"

namespace Zap
{
class Ship;

class ZoneControlGameType : public GameType
{
   typedef GameType Parent;

   Vector<GoalZone*> mZones;
public:
   void shipTouchFlag(Ship *theShip, FlagItem *theFlag);
   void flagDropped(Ship *theShip, FlagItem *theFlag);
   void addZone(GoalZone *z);
   void shipTouchZone(Ship *s, GoalZone *z);
   const char *getGameTypeString() { return "Zone Control"; }
   const char *getInstructionString() { return "Carry the flag into each of the capture zones!"; }

   TNL_DECLARE_CLASS(ZoneControlGameType);
};

TNL_IMPLEMENT_NETOBJECT(ZoneControlGameType);


void ZoneControlGameType::shipTouchFlag(Ship *theShip, FlagItem *theFlag)
{
   GameConnection *controlConnection = theShip->getControllingClient();
   ClientRef *cl = controlConnection->getClientRef();
   if(!cl)
      return;

   static StringTableEntry takeString("%e0 of team %e1 has the flag!");
   Vector<StringTableEntry> e;
   e.push_back(cl->name);
   e.push_back(mTeams[cl->teamId].name);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
   theFlag->mountToShip(theShip);
}

void ZoneControlGameType::flagDropped(Ship *theShip, FlagItem *theFlag)
{
   static StringTableEntry dropString("%e0 dropped the flag!");
   Vector<StringTableEntry> e;
   e.push_back(theShip->mPlayerName);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagDrop, dropString, e);
}

void ZoneControlGameType::addZone(GoalZone *z)
{
   mZones.push_back(z);
}

void ZoneControlGameType::shipTouchZone(Ship *s, GoalZone *z)
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
         mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
      setTeamScore(oldTeam, mTeams[oldTeam].score - 1);
   }
   else
   {
      static StringTableEntry takeString("%e0 captured an unclaimed zone!");
      Vector<StringTableEntry> e;
      e.push_back(s->mPlayerName);
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
   }
   setTeamScore(s->getTeam(), mTeams[s->getTeam()].score + 1);
   z->setTeam(s->getTeam());
   for(S32 i = 0; i < mZones.size(); i++)
   {
      if(mZones[i]->getTeam() != s->getTeam())
         return;
   }
   
   static StringTableEntry tdString("Team %e0 scored a touchdown!");
   Vector<StringTableEntry> e;
   e.push_back(mTeams[s->getTeam()].name);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagCapture, tdString, e);

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