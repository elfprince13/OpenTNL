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

TNL_IMPLEMENT_NETOBJECT(SoccerGameType);

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

TNL_IMPLEMENT_NETOBJECT(SoccerBallItem);

SoccerBallItem::SoccerBallItem(Point pos) : Item(pos, true, 30, 4)
{
   mObjectTypeMask |= CommandMapVisType | TurretTargetType;
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
   glColor3f(1, 1, 1);
   drawCircle(pos, mRadius);
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
         g->scoreGoal(lastPlayerTouch, goal->getTeam());
         mSendHomeTimer.reset(1500);
      }
   }
   return true;
}

TNL_IMPLEMENT_NETOBJECT(SoccerGoalObject);

SoccerGoalObject::SoccerGoalObject()
{
   mTeam = 0;
   mObjectTypeMask |= CommandMapVisType;
   mNetFlags.set(Ghostable);
}

void SoccerGoalObject::onAddedToGame(Game *theGame)
{
   if(!isGhost())
      setScopeAlways();
}

void SoccerGoalObject::render()
{
   F32 alpha = 0.5;
   Color theColor = getGame()->getGameType()->mTeams[getTeam()].color;
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

   mTeam = atoi(argv[0]);
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
   stream->write(mTeam);
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
   stream->read(&mTeam);
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
