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

#include "CTFGame.h"
#include "ship.h"
#include "UIGame.h"
#include "flagItem.h"
#include "sfx.h"

#include "glutInclude.h"
#include <stdio.h>

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(CTFGameType);

void CTFGameType::shipTouchFlag(Ship *theShip, FlagItem *theFlag)
{
   GameConnection *controlConnection = theShip->getControllingClient();
   S32 clientIndex = findClientIndexByConnection(controlConnection);

   if(clientIndex == -1)
      return;

   ClientRef &cl = mClientList[clientIndex];

   if(cl.teamId == theFlag->getTeam())
   {
      if(!theFlag->isAtHome())
      {
         static StringTableEntry returnString("%e0 returned the %e1 flag.");
         Vector<StringTableEntry> e;
         e.push_back(cl.name);
         e.push_back(mTeams[theFlag->getTeam()].name);
         for(S32 i = 0; i < mClientList.size(); i++)
            mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagReturn, returnString, e);

         theFlag->sendHome();
         cl.score += ReturnScore;
      }
      else
      {
         // check if this client has an enemy flag mounted
         for(S32 i = 0; i < theShip->mMountedItems.size(); i++)
         {
            Item *theItem = theShip->mMountedItems[i];
            FlagItem *mountedFlag = dynamic_cast<FlagItem *>(theItem);
            if(mountedFlag)
            {
               setTeamScore(cl.teamId, mTeams[cl.teamId].score + 1);

               static StringTableEntry capString("%e0 captured the %e1 flag!");
               Vector<StringTableEntry> e;
               e.push_back(cl.name);
               e.push_back(mTeams[mountedFlag->getTeam()].name);
               for(S32 i = 0; i < mClientList.size(); i++)
                  mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagCapture, capString, e);

               // score the flag for the client's team...
               mountedFlag->dismount();
               mountedFlag->sendHome();
               cl.score += CapScore;
            }
         }
      }
   }
   else
   {
      static StringTableEntry takeString("%e0 took the %e1 flag!");
      Vector<StringTableEntry> e;
      e.push_back(cl.name);
      e.push_back(mTeams[theFlag->getTeam()].name);
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagSnatch, takeString, e);
      theFlag->mountToShip(theShip);
   }
}

void CTFGameType::flagDropped(const StringTableEntry &playerName, S32 flagTeamIndex)
{
   static StringTableEntry dropString("%e0 dropped the %e1 flag!");
   Vector<StringTableEntry> e;
   e.push_back(playerName);
   e.push_back(mTeams[flagTeamIndex].name);
   for(S32 i = 0; i < mClientList.size(); i++)
      mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagDrop, dropString, e);
}

};

