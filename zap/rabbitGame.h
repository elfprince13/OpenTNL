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

#ifndef _RABBITGAME_H_
#define _RABBITGAME_H_

#include "gameType.h"
#include "item.h"
#include "gameWeapons.h"
#include "shipItems.h"

namespace Zap
{
class Ship;
class RabbitFlagItem;

class RabbitGameType : public GameType
{
   typedef GameType Parent;

   S32 mScoreLimit;

   Vector<U32> mRabbitLoadout;
   Vector<U32> mHunterLoadout;
public:

   enum
   {
      RabbitMsgGrab,
      RabbitMsgRabbitKill,
      RabbitMsgRabbitDead,
      RabbitMsgDrop,
      RabbitMsgReturn,
      RabbitMsgGameOverWin,
      RabbitMsgGameOverTie
   };

   enum
   {
      RabbitKillBonus = 4,    //one for the kill and 4 more = 5 point bonus
      RabbidRabbitBonus = 4
   };

   RabbitGameType()
   {
      mScoreLimit = 100;

      mRabbitLoadout.push_back(ModuleBoost);
      mRabbitLoadout.push_back(ModuleShield);
      mRabbitLoadout.push_back(WeaponBounce);
      mRabbitLoadout.push_back(WeaponTriple);
      mRabbitLoadout.push_back(WeaponBurst);

      mHunterLoadout.push_back(ModuleSensor);
      mHunterLoadout.push_back(ModuleCloak);
      mHunterLoadout.push_back(WeaponPhaser);
      mHunterLoadout.push_back(WeaponBounce);
      mHunterLoadout.push_back(WeaponTriple);
   }
   
   void processArguments(S32 argc, const char **argv);
   void spawnShip(GameConnection *theClient);

   bool objectCanDamageObject(GameObject *damager, GameObject *victim);
   void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);
   bool shipHasFlag(Ship *ship);

   void onFlagGrabbed(Ship *ship, RabbitFlagItem *flag);
   void onFlagHeld(Ship *ship);
   void onFlagDropped(Ship *victimShip);
   void onFlaggerDead(Ship *killerShip);
   void onFlaggerKill(Ship *rabbitShip);
   void onFlagReturned();

   void onClientScore(Ship *ship, S32 howMuch);
   const char *getGameTypeString() { return "Rabbit"; }
   const char *getInstructionString() { return "Grab the flag and hold it for as long as you can!"; }

   TNL_DECLARE_RPC(s2cRabbitMessage, (U32 msgIndex, StringTableEntryRef clientName));
   TNL_DECLARE_CLASS(RabbitGameType);
};

class RabbitFlagItem : public Item
{
   typedef Item Parent;

   Timer mReturnTimer;
   Timer mScoreTimer;

   Point initialPos;

public:
   RabbitFlagItem(Point pos = Point());
   void processArguments(S32 argc, const char **argv);
   void onAddedToGame(Game *theGame);

   void renderItem(Point pos);
   bool collide(GameObject *hitObject);

   void idle(GameObject::IdleCallPath path);
   void onMountDestroyed();

   void sendHome();


   TNL_DECLARE_CLASS(RabbitFlagItem);

};

};


#endif

