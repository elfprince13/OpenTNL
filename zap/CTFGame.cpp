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

namespace Zap
{

static void renderFlag(Point pos, Color c)
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
   if(scoreboardVisible)
   {
      U32 totalWidth = 780;
      U32 teamWidth = totalWidth / mTeams.size();
      U32 maxTeamPlayers = 0;
      countTeamPlayers();

      for(S32 i = 0; i < mTeams.size(); i++)
         if(mTeams[i].numPlayers > maxTeamPlayers)
            maxTeamPlayers = mTeams[i].numPlayers;

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
         U32 fontSize = maxHeight * 0.8f;
         for(S32 j = 0; j < mClientList.size(); j++)
         {
            if(mClientList[j].teamId == i)
            {
               UserInterface::drawString(xl + 40, curRowY, fontSize, mClientList[i].name.getString());
               UserInterface::drawStringf(xr - 140, curRowY, fontSize, "%d", mClientList[i].score);
               UserInterface::drawStringf(xr - 70, curRowY, fontSize, "%d", mClientList[i].ping);
               curRowY += maxHeight;
            }           
         }
      }
   }
   else
   {
      for(S32 i = 0; i < mTeams.size(); i++)
      {
         Point pos(750, 550 - i * 38);
         renderFlag(pos + Point(-20, 18), mTeams[i].color);
         glColor3f(1,1,1);
         UserInterface::drawStringf(pos.x, pos.y, 32, "%d", mTeams[i].score);
      }
   }
}

Vector<RangedU32<0, CTFGameType::MaxPing> > CTFGameType::mPingTimes; ///< Static vector used for constructing update RPCs
Vector<Int<9> > CTFGameType::mScores;

void CTFGameType::updateClientScoreboard(S32 clientIndex)
{
   mPingTimes.clear();
   mScores.clear();

   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(mClientList[i].ping < MaxPing)
         mPingTimes.push_back(mClientList[i].ping);
      else
         mPingTimes.push_back(MaxPing);
      mScores.push_back(mClientList[i].score);
   }

   NetObject::setRPCDestConnection(mClientList[clientIndex].clientConnection);
   s2cScoreboardUpdate(mPingTimes, mScores);
   NetObject::setRPCDestConnection(NULL);
}

TNL_DECLARE_RPC_MEM_ENUM(CTFGameType, MaxPing);

TNL_IMPLEMENT_NETOBJECT_RPC(CTFGameType, s2cScoreboardUpdate, (const Vector<RangedU32<0, CTFGameType::MaxPing> > &pingTimes, const Vector<Int<9> > &scores),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(i >= pingTimes.size())
         break;

      mClientList[i].ping = pingTimes[i];
      mClientList[i].score = scores[i];
   }
}

void CTFGameType::shipTouchFlag(Ship *theShip, CTFFlagItem *theFlag)
{
   GameConnection *controlConnection = theShip->getControllingClient();
   S32 clientIndex = findClientIndexByConnection(controlConnection);

   if(clientIndex == -1)
      return;

   ClientRef &cl = mClientList[clientIndex];

   if(cl.teamId == theFlag->getTeamIndex())
   {
      if(!theFlag->isAtHome())
      {
         s2cCTFMessage(CTFMsgReturnFlag, cl.clientId, theFlag->getTeamIndex());
         theFlag->sendHome();
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
               mTeams[cl.teamId].score++;
               s2cSetTeamScore(cl.teamId, mTeams[cl.teamId].score);
               s2cCTFMessage(CTFMsgCaptureFlag, cl.clientId, mountedFlag->getTeamIndex());
               // score the flag for the client's team...
               mountedFlag->dismount();
               mountedFlag->sendHome();
            }
         }
      }
   }
   else
   {
      s2cCTFMessage(CTFMsgTakeFlag, cl.clientId, theFlag->getTeamIndex());
      theFlag->mountToShip(theShip);
   }
}

static const char *CTFMessages[] = 
{
  "%s returned the %s flag.",
  "%s captured the %s flag!",
  "%s took the %s flag!",
};

static U32 CTFFlagSounds[] = 
{
   SFXFlagReturn,
   SFXFlagCapture,
   SFXFlagSnatch,
};

TNL_IMPLEMENT_NETOBJECT_RPC(CTFGameType, s2cCTFMessage, (U32 messageIndex, U32 clientId, U32 teamIndex),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexById(clientId);

   gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
            CTFMessages[messageIndex], 
               mClientList[clientIndex].name.getString(),
               mTeams[teamIndex].name.getString());
   SFXObject::play(CTFFlagSounds[messageIndex]);
}

TNL_IMPLEMENT_NETOBJECT(CTFFlagItem);

CTFFlagItem::CTFFlagItem(Point pos) : Item(pos, false, 20)
{
   teamIndex = 0;
   mNetFlags.set(Ghostable);
}

void CTFFlagItem::processArguments(S32 argc, const char **argv)
{
   if(argc < 3)
      return;

   teamIndex = atoi(argv[0]);
   Parent::processArguments(argc-1, argv+1);
   initialPos = mMoveState[ActualState].pos;
}

U32 CTFFlagItem::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   if(stream->writeFlag(updateMask & InitialMask))
      stream->writeInt(teamIndex, 4);
   return Parent::packUpdate(connection, updateMask, stream);
}

void CTFFlagItem::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   if(stream->readFlag())
      teamIndex = stream->readInt(4);
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

   c = gt->mTeams[teamIndex].color;

   renderFlag(pos + offset, c);
}

bool CTFFlagItem::collide(GameObject *hitObject)
{
   if(isGhost() || mIsMounted)
      return false;

   if(!(hitObject->getObjectTypeMask() & ShipType))
      return false;

   CTFGameType *theGameType = dynamic_cast<CTFGameType *>(getGame()->getGameType());
   if(theGameType)
      theGameType->shipTouchFlag((Ship *) hitObject, this);
   return false;
}

};