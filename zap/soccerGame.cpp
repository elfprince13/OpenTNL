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

#include "soccerGame.h"
#include "glutInclude.h"
#include "UIGame.h"
#include "sfx.h"
#include "gameNetInterface.h"
#include "ship.h"

namespace Zap
{

class Ship;
class SoccerBallItem;

extern void renderFlag(Point pos, Color c);

TNL_IMPLEMENT_NETOBJECT(SoccerGameType);

void SoccerGameType::renderInterfaceOverlay(bool scoreboardVisible)
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
         U32 fontSize = maxHeight * 0.8f;
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
         UserInterface::drawStringf(pos.x, pos.y, 32, "%d", mTeams[i].score);
      }
   }
   renderTimeLeft();
   renderTalkingClients();
}

void SoccerGameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
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

bool SoccerGameType::objectCanDamageObject(GameObject *damager, GameObject *victim)
{
   GameConnection *c1 = (damager ? damager->getControllingClient() : NULL);
   GameConnection *c2 = (victim ? victim->getControllingClient() : NULL);

   if(!c1 || !c2)
      return true;

   return mClientList[findClientIndexByConnection(c1)].teamId !=
          mClientList[findClientIndexByConnection(c2)].teamId;
}

TNL_IMPLEMENT_NETOBJECT_RPC(SoccerGameType, s2cSoccerScoreMessage, (U32 msgIndex, StringTableEntryRef clientName, U32 teamIndex),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   if(msgIndex == SoccerMsgScoreGoal)
   {
      SFXObject::play(SFXFlagCapture);
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                  "%s scored a goal on team %s!", 
                  clientName.getString(),
                  mTeams[teamIndex].name.getString());
   }
   else if(msgIndex == SoccerMsgScoreOwnGoal)
   {
      SFXObject::play(SFXFlagCapture);
      if(clientName.isNull())
         gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "A goal was scored on team %s!", 
                     mTeams[teamIndex].name.getString());
      else
         gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "%s scored a goal for the other team%s!", 
                     clientName.getString(),
                     mTeams.size() > 2 ? "s" : "");
   }
   else if(msgIndex == SoccerMsgGameOverTeamWin)
   {
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "Team %s wins the game!", 
                     mTeams[teamIndex].name.getString());
      SFXObject::play(SFXFlagCapture);
   }
   else if(msgIndex == SoccerMsgGameOverTie)
   {
      gGameUserInterface.displayMessage(Color(0.6f, 1.0f, 0.8f), 
                     "The game ended in a tie.");
      SFXObject::play(SFXFlagDrop);
   }
}

void SoccerGameType::scoreGoal(StringTableEntry playerName, U32 goalTeamIndex)
{
   S32 index = findClientIndexByName(playerName);
   S32 scoringTeam = -1;
   if(index != -1)
      scoringTeam = mClientList[index].teamId;

   if(scoringTeam == -1 || scoringTeam == goalTeamIndex)
   {
      // give all the other teams a point.
      for(S32 i = 0; i < mTeams.size(); i++)
      {
         if(i != goalTeamIndex)
            setTeamScore(i, mTeams[i].score + 1);
      }
      s2cSoccerScoreMessage(SoccerMsgScoreOwnGoal, playerName, goalTeamIndex);
   }
   else
   {
      mClientList[index].score += GoalScore;
      setTeamScore(scoringTeam, mTeams[scoringTeam].score + 1);
      s2cSoccerScoreMessage(SoccerMsgScoreGoal, playerName, goalTeamIndex);
   }
}

void SoccerGameType::gameOverManGameOver()
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
      s2cSoccerScoreMessage(SoccerMsgGameOverTie, StringTableEntry(), 0);
   else
      s2cSoccerScoreMessage(SoccerMsgGameOverTeamWin, StringTableEntry(), teamWinner);
}

TNL_IMPLEMENT_NETOBJECT(SoccerBallItem);

SoccerBallItem::SoccerBallItem(Point pos) : Item(pos, true, 30, 4)
{
   mObjectTypeMask |= CommandMapVisType;
   mNetFlags.set(Ghostable);
   initialPos = pos;
}

void SoccerBallItem::processArguments(S32 argc, const char **argv)
{
   Parent::processArguments(argc, argv);
   initialPos = mMoveState[ActualState].pos;
}

void SoccerBallItem::renderItem(Point pos)
{
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);

   glColor3f(1, 1, 1);
   glBegin(GL_LINE_LOOP);

   for(F32 theta = 0; theta < 2 * 3.1415; theta += 0.2)
      glVertex2f(cos(theta) * mRadius, sin(theta) * mRadius);

   glEnd();
   glPopMatrix();
}

void SoccerBallItem::idle(GameObject::IdleCallPath path)
{
   if(mSendHomeTimer.update(mCurrentMove.time))
      sendHome();
   else if(mSendHomeTimer.getCurrent())
   {
      F32 accelFraction = 1 - (0.98 * mCurrentMove.time * 0.001f);

      mMoveState[ActualState].vel *= accelFraction;
      mMoveState[RenderState].vel *= accelFraction;
   }
   Parent::idle(path);
}

void SoccerBallItem::damageObject(DamageInfo *theInfo)
{
   // compute impulse direction
   Point dv = theInfo->impulseVector - mMoveState[ActualState].vel;
   Point iv = mMoveState[ActualState].pos - theInfo->collisionPoint;
   iv.normalize();
   mMoveState[ActualState].vel += iv * dv.dot(iv) * 0.3;
   if(theInfo->damagingObject && (theInfo->damagingObject->getObjectTypeMask() & ShipType))
   {
      lastPlayerTouch = ((Ship *) theInfo->damagingObject)->mPlayerName;
   }
}

void SoccerBallItem::sendHome()
{
   mMoveState[ActualState].vel = mMoveState[RenderState].vel = Point();
   mMoveState[ActualState].pos = mMoveState[RenderState].pos = initialPos;
   setMaskBits(PositionMask);
   updateExtent();
}

bool SoccerBallItem::collide(GameObject *hitObject)
{
   if(isGhost())
      return true;

   if(hitObject->getObjectTypeMask() & ShipType)
   {
      lastPlayerTouch = ((Ship *) hitObject)->mPlayerName;
   }
   else
   {
      SoccerGoalObject *goal = dynamic_cast<SoccerGoalObject *>(hitObject);

      if(goal && !mSendHomeTimer.getCurrent())
      {
         SoccerGameType *g = (SoccerGameType *) getGame()->getGameType();
         g->scoreGoal(lastPlayerTouch, goal->getTeamIndex());
         mSendHomeTimer.reset(1500);
      }
   }
   return true;
}

TNL_IMPLEMENT_NETOBJECT(SoccerGoalObject);

SoccerGoalObject::SoccerGoalObject()
{
   teamIndex = 0;
   mObjectTypeMask |= CommandMapVisType;
   mNetFlags.set(Ghostable);
}

void SoccerGoalObject::onAddedToGame(Game *theGame)
{
   if(!isGhost())
   {
      setInterface(theGame->getNetInterface());
      setScopeAlways();
   }
}

void SoccerGoalObject::render()
{
   F32 alpha = 0.5;
   Color theColor = getGame()->getGameType()->mTeams[teamIndex].color;
   glColor3f(theColor.r * alpha, theColor.g * alpha, theColor.b * alpha);
   glBegin(GL_POLYGON);
   for(S32 i = 0; i < mPolyBounds.size(); i++)
      glVertex2f(mPolyBounds[i].x, mPolyBounds[i].y);
   glEnd();
}

bool SoccerGoalObject::getCollisionPoly(Vector<Point> &polyPoints)
{
   for(S32 i = 0; i < mPolyBounds.size(); i++)
      polyPoints.push_back(mPolyBounds[i]);
   return true;
}

void SoccerGoalObject::processArguments(S32 argc, const char **argv)
{
   if(argc < 7)
      return;

   teamIndex = atoi(argv[0]);
   for(S32 i = 1; i < argc; i += 2)
   {
      Point p;
      p.x = atof(argv[i]) * getGame()->getGridSize();
      p.y = atof(argv[i+1]) * getGame()->getGridSize();
      mPolyBounds.push_back(p);
   }
   computeExtent();
}

void SoccerGoalObject::computeExtent()
{
   Rect extent(mPolyBounds[0], mPolyBounds[0]);
   for(S32 i = 1; i < mPolyBounds.size(); i++)
      extent.unionPoint(mPolyBounds[i]);
   setExtent(extent);
}

bool SoccerGoalObject::collide(GameObject *hitObject)
{
   return false;
}

U32 SoccerGoalObject::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   stream->write(teamIndex);
   stream->writeRangedU32(mPolyBounds.size(), 0, MaxPoints);
   for(S32 i = 0; i < mPolyBounds.size(); i++)
   {
      stream->write(mPolyBounds[i].x);
      stream->write(mPolyBounds[i].y);
   }
   return 0;
}

void SoccerGoalObject::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   stream->read(&teamIndex);
   S32 size = stream->readRangedU32(0, MaxPoints);
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

};
