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

   void onFlagGrabbed(Ship *ship, RabbitFlagItem *flag);
   void onFlagHeld(Ship *ship);
   void onFlagDropped(Ship *victimShip);
   void onFlaggerDead(Ship *killerShip);
   void onFlaggerKill(Ship *rabbitShip);
   void onFlagReturned();

   void onClientScore(Ship *ship, S32 howMuch);

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

