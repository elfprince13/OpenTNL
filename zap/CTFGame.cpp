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
#include "sfx.h"

#include "glutInclude.h"
#include <stdio.h>

namespace Zap
{

void renderFlag(Point pos, Color c)
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

TNL_IMPLEMENT_NETOBJECT(CTFGameType);

void CTFGameType::renderInterfaceOverlay(bool scoreboardVisible)
{
   if((mGameOver || scoreboardVisible) && mTeams.size() > 0)
   {
      U32 totalWidth = 780;
      U32 teamWidth = totalWidth / mTeams.size();
      U32 maxTeamPlayers = 0;
      countTeamPlayers();

      for(S32 i = 0; i < mTeams.size(); i++)
         if(mTeams[i].numPlayers > maxTeamPlayers)
            maxTeamPlayers = mTeams[i].numPlayers;

      if(!maxTeamPlayers)
         return;

      U32 totalHeight = 580;
      U32 maxHeight = (totalHeight - 40) / maxTeamPlayers;
      if(maxHeight > 30)
         maxHeight = 30;

      totalHeight = 40 + maxHeight * maxTeamPlayers;
      U32 yt = (600 - totalHeight) / 2;
      U32 yb = yt + totalHeight;

      for(S32 i = 0; i < mTeams.size(); i++)
      {
         U32 xl = 10 + i * teamWidth;
         U32 xr = xl + teamWidth - 2;

         Color c = mTeams[i].color;
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

         glColor4f(c.r, c.g, c.b, 0.6);
         glBegin(GL_POLYGON);
         glVertex2f(xl, yt);
         glVertex2f(xr, yt);
         glVertex2f(xr, yb);
         glVertex2f(xl, yb);
         glEnd();

         glDisable(GL_BLEND);
         glBlendFunc(GL_ONE, GL_ZERO);

         renderFlag(Point(xl + 20, yt + 18), c);
         renderFlag(Point(xr - 20, yt + 18), c);

         glColor3f(1,1,1);
         glBegin(GL_LINES);
         glVertex2f(xl, yt + 40);
         glVertex2f(xr, yt + 40);
         glEnd();

         UserInterface::drawString(xl + 40, yt + 2, 30, mTeams[i].name.getString());

         UserInterface::drawStringf(xr - 140, yt + 2, 30, "%d", mTeams[i].score);

         U32 curRowY = yt + 41;
         U32 fontSize = U32(maxHeight * 0.8f);
         for(S32 j = 0; j < mClientList.size(); j++)
         {
            if(mClientList[j].teamId == i)
            {
               UserInterface::drawString(xl + 40, curRowY, fontSize, mClientList[j].name.getString());

               static char buff[255] = "";
               dSprintf(buff, sizeof(buff), "%d", mClientList[j].score);

               UserInterface::drawString(xr - (120 + UserInterface::getStringWidth(fontSize, buff)), curRowY, fontSize, buff);
               UserInterface::drawStringf(xr - 70, curRowY, fontSize, "%d", mClientList[j].ping);
               curRowY += maxHeight;
            }
         }
      }
   }
   else
   {
      for(S32 i = 0; i < mTeams.size(); i++)
      {
         Point pos(750, 535 - i * 38);
         renderFlag(pos + Point(-20, 18), mTeams[i].color);
         glColor3f(1,1,1);
         UserInterface::drawStringf(U32(pos.x), U32(pos.y), 32, "%d", mTeams[i].score);
      }
   }
   renderTimeLeft();
   renderTalkingClients();
}

void CTFGameType::shipTouchFlag(Ship *theShip, CTFFlagItem *theFlag)
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
            CTFFlagItem *mountedFlag = dynamic_cast<CTFFlagItem *>(theItem);
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

bool CTFGameType::objectCanDamageObject(GameObject *damager, GameObject *victim)
{
   if(damager == victim)
      return true;

   GameConnection *c1 = (damager ? damager->getControllingClient() : NULL);
   GameConnection *c2 = (victim ? victim->getControllingClient() : NULL);

   if(!c1 || !c2)
      return true;

   return mClientList[findClientIndexByConnection(c1)].teamId !=
          mClientList[findClientIndexByConnection(c2)].teamId;
}

void CTFGameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   GameConnection *killer = killerObject ? killerObject->getControllingClient() : NULL;
   S32 killerIndex = findClientIndexByConnection(killer);
   S32 clientIndex = findClientIndexByConnection(theClient);

   if(killerIndex != -1)
   {
      // Punish team killers slightly
      if(mClientList[killerIndex].teamId == mClientList[clientIndex].teamId)
         mClientList[killerIndex].score -= KillScore/4;
      else
         mClientList[killerIndex].score += KillScore;

      s2cKillMessage(mClientList[clientIndex].name, mClientList[killerIndex].name);
   }
   mClientList[clientIndex].respawnTimer.reset(RespawnDelay);
}

void CTFGameType::gameOverManGameOver()
{
   Parent::gameOverManGameOver();
   bool tied = false;
   S32 teamWinner = 0;
   U32 winningScore = mTeams[0].score;
   for(S32 i = 1; i < mTeams.size(); i++)
   {
      if(mTeams[i].score == winningScore)
         tied = true;
      else if(mTeams[i].score > winningScore)
      {
         teamWinner = i;
         winningScore = mTeams[i].score;
         tied = false;
      }
   }
   if(tied)
   {
      static StringTableEntry tieMessage("The game ended in a tie.");
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].clientConnection->s2cDisplayMessage(GameConnection::ColorNuclearGreen, SFXFlagDrop, tieMessage);
   }
   else
   {
      static StringTableEntry winMessage("Team %e0 wins the game!");
      Vector<StringTableEntry> e;
      e.push_back(mTeams[teamWinner].name);
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagCapture, winMessage, e);
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

TNL_IMPLEMENT_NETOBJECT(CTFFlagItem);

CTFFlagItem::CTFFlagItem(Point pos) : Item(pos, false, 20)
{
   mTeam = 0;
   mNetFlags.set(Ghostable);
   mObjectTypeMask |= CommandMapVisType;
}

void CTFFlagItem::processArguments(S32 argc, const char **argv)
{
   if(argc < 3)
      return;

   mTeam = atoi(argv[0]);
   Parent::processArguments(argc-1, argv+1);
   initialPos = mMoveState[ActualState].pos;
}

U32 CTFFlagItem::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   if(stream->writeFlag(updateMask & InitialMask))
      stream->writeInt(mTeam, 4);
   return Parent::packUpdate(connection, updateMask, stream);
}

void CTFFlagItem::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   if(stream->readFlag())
      mTeam = stream->readInt(4);
   Parent::unpackUpdate(connection, stream);
}

bool CTFFlagItem::isAtHome()
{
   return mMoveState[ActualState].pos == initialPos;
}

void CTFFlagItem::sendHome()
{
   mMoveState[ActualState].pos = mMoveState[RenderState].pos = initialPos;
   setMaskBits(PositionMask);
   updateExtent();
}

void CTFFlagItem::renderItem(Point pos)
{
   Point offset;

   if(mIsMounted)
      offset.set(15, -15);

   Color c;
   GameType *gt = getGame()->getGameType();

   c = gt->mTeams[getTeam()].color;

   renderFlag(pos + offset, c);
}

bool CTFFlagItem::collide(GameObject *hitObject)
{
   if(isGhost() || mIsMounted)
      return false;

   if(!(hitObject->getObjectTypeMask() & ShipType))
      return false;

   if(((Ship *) hitObject)->hasExploded)
      return false;

   CTFGameType *theGameType = dynamic_cast<CTFGameType *>(getGame()->getGameType());
   if(theGameType)
      theGameType->shipTouchFlag((Ship *) hitObject, this);
   return false;
}

void CTFFlagItem::onMountDestroyed()
{
   CTFGameType *gt = dynamic_cast<CTFGameType *>(getGame()->getGameType());
   if(!gt)
      return;

   if(!mMount.isValid())
      return;

   gt->flagDropped(mMount->mPlayerName, getTeam());
   dismount();
}

};

