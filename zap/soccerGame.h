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

#ifndef _SOCCERGAME_H_
#define _SOCCERGAME_H_

#include "gameType.h"
#include "item.h"

namespace Zap
{

class Ship;
class SoccerBallItem;

class SoccerGameType : public GameType
{
   typedef GameType Parent;
   enum Scores
   {
      KillScore    = 100,
      GoalScore    = 500,
   };
public:
   void renderInterfaceOverlay(bool scoreboardVisible);
   void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);
   void scoreGoal(StringTableEntry playerName, U32 goalTeamIndex);
   void gameOverManGameOver();

   enum {
      SoccerMsgScoreGoal,
      SoccerMsgScoreOwnGoal,
      SoccerMsgGameOverTeamWin,
      SoccerMsgGameOverTie,
   };

   TNL_DECLARE_RPC(s2cSoccerScoreMessage, (U32 msgIndex, StringTableEntryRef clientName, U32 teamIndex));
   TNL_DECLARE_CLASS(SoccerGameType);
};

class SoccerBallItem : public Item
{
   typedef Item Parent;
   Point initialPos;
   Timer mSendHomeTimer;
   StringTableEntry lastPlayerTouch;
public:
   SoccerBallItem(Point pos = Point());
   void renderItem(Point pos);
   void sendHome();
   void damageObject(DamageInfo *theInfo);
   void idle(GameObject::IdleCallPath path);
   void processArguments(S32 argc, const char **argv);

   bool collide(GameObject *hitObject);

   TNL_DECLARE_CLASS(SoccerBallItem);
};

class SoccerGoalObject : public GameObject
{
   typedef GameObject Parent;
   Vector<Point> mPolyBounds;
   enum {
      MaxPoints = 10,
   };
public:
   SoccerGoalObject();

   void render();
   S32 getRenderSortValue() { return -1; }
   void processArguments(S32 argc, const char **argv);
   void onAddedToGame(Game *theGame);
   void computeExtent();
   bool getCollisionPoly(Vector<Point> &polyPoints);

   bool collide(GameObject *hitObject);

   U32 packUpdate(GhostConnection *connection, U32 mask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   TNL_DECLARE_CLASS(SoccerGoalObject);
};

};


#endif

