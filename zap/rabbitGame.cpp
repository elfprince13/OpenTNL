#include "rabbitGame.h"
#include "ship.h"
#include "UIGame.h"
#include "sfx.h"

#include "glutInclude.h"
#include <stdio.h>

namespace Zap
{

void renderRabbitFlag(Point pos, Color c)
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

TNL_IMPLEMENT_NETOBJECT_RPC(RabbitGameType, s2cRabbitMessage, (U32 msgIndex, StringTableEntryRef clientName),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   switch (msgIndex)
   {
   case RabbitMsgGrab:
      SFXObject::play(SFXFlagCapture);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s GRABBED the Carrot!",
                  clientName.getString());
      break;
   case RabbitMsgRabbitKill:
      SFXObject::play(SFXShipHeal);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s is a rabbid rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgDrop:
      SFXObject::play(SFXFlagDrop);
      gGameUserInterface.displayMessage(Color(0.0f, 1.0f, 0.0f),
                  "%s DROPPED the Carrot!",
                  clientName.getString());
      break;
   case RabbitMsgRabbitDead:
      SFXObject::play(SFXShipExplode);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s killed the rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgReturn:
      SFXObject::play(SFXFlagReturn);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 1.0f),
                  "The Carrot has been returned!");
      break;
   case RabbitMsgGameOverWin:
      gGameUserInterface.displayMessage(Color(1.0f, 1.0f, 0.0f),
                  "%s is the top rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgGameOverTie:
      gGameUserInterface.displayMessage(Color(1.0f, 1.0f, 0.0f),
                  "No top rabbit - Carrot wins by default!");
      break;
   }
}

//-----------------------------------------------------
// RabbitGameType
//-----------------------------------------------------
TNL_IMPLEMENT_NETOBJECT(RabbitGameType);

void RabbitGameType::processArguments(S32 argc, const char **argv)
{
   if (argc != 2)
      return;

   mScoreLimit = atoi(argv[0]);
   Parent::processArguments(argc-1, argv+1);
}

void RabbitGameType::spawnShip(GameConnection *theClient)
{
   Parent::spawnShip(theClient);
   S32 clientIndex = findClientIndexByConnection(theClient);
   setClientShipLoadout(clientIndex, mHunterLoadout);
}

bool RabbitGameType::objectCanDamageObject(GameObject *damager, GameObject *victim)
{
   if(!damager)
      return true;

   //if one of them isn't a ship, default to whatever the parent is
   if(!(damager->getObjectTypeMask() & victim->getObjectTypeMask() & ShipType))
      return Parent::objectCanDamageObject(damager, victim);

   GameConnection *c1 = damager->getOwner();
   GameConnection *c2 = victim->getOwner();

   if( (!c1 || !c2) || (c1 == c2))
      return true;

   Ship *damnShip = (Ship *) c1->getControlObject();
   Ship *victimShip = (Ship *) c2->getControlObject();

   if(!damnShip || !victimShip)
      return true;

   //only hunters can hurt rabbits and only rabbits can hurt hunters
   return shipHasFlag(damnShip) != shipHasFlag(victimShip);
}

bool RabbitGameType::shipHasFlag(Ship *ship)
{
   if (!ship)
      return false;

   for (S32 k = 0; k < ship->mMountedItems.size(); k++)
   {
      if (RabbitFlagItem *flag = dynamic_cast<RabbitFlagItem *>(ship->mMountedItems[k].getPointer()))
         return true;
   }

   return false;
}

void RabbitGameType::onClientScore(Ship *ship, S32 howMuch)
{
   GameConnection *controlConnection = ship->getControllingClient();
   S32 clientIndex = findClientIndexByConnection(controlConnection);

   if(clientIndex == -1)
      return;

   ClientRef &cl = mClientList[clientIndex];
   cl.score += howMuch;
   if (cl.score >= mScoreLimit)
      gameOverManGameOver();
}

void RabbitGameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   Parent::controlObjectForClientKilled(theClient, clientObject, killerObject);

   Ship *killerShip = NULL;
   GameConnection *ko = killerObject->getOwner();
   if(ko)
      killerShip = (Ship *) ko->getControlObject();

   Ship *victimShip = dynamic_cast<Ship *>(clientObject);

   if (killerShip)
   {
      if (shipHasFlag(killerShip))
      {
         //rabbit killed another person
         onFlaggerKill(killerShip);
      }
      else if (shipHasFlag(victimShip))
      {
         //someone killed the rabbit!  Poor rabbit!
         onFlaggerDead(killerShip);
      }
   }
}

void RabbitGameType::onFlagGrabbed(Ship *ship, RabbitFlagItem *flag)
{
   s2cRabbitMessage(RabbitMsgGrab, ship->mPlayerName.getString());

   flag->mountToShip(ship);
   S32 clientIndex = findClientIndexByConnection(ship->getControllingClient());
   setClientShipLoadout(clientIndex, mRabbitLoadout);
}

void RabbitGameType::onFlagHeld(Ship *ship)
{
   onClientScore(ship, 1);
}

void RabbitGameType::onFlagDropped(Ship *victimShip)
{
   s2cRabbitMessage(RabbitMsgDrop, victimShip->mPlayerName.getString());
}

void RabbitGameType::onFlaggerKill(Ship *rabbitShip)
{
   s2cRabbitMessage(RabbitMsgRabbitKill, rabbitShip->mPlayerName.getString());
   onClientScore(rabbitShip, RabbidRabbitBonus);
}

void RabbitGameType::onFlaggerDead(Ship *killerShip)
{
   s2cRabbitMessage(RabbitMsgRabbitDead, killerShip->mPlayerName.getString());
   onClientScore(killerShip, RabbitKillBonus);
}

void RabbitGameType::onFlagReturned()
{
   static StringTableEntry returnString("The carrot has been returned!");
   for (S32 i = 0; i < mClientList.size(); i++)
      mClientList[i].clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagReturn, returnString, NULL);
}

//-----------------------------------------------------
// RabbitFlagItem
//-----------------------------------------------------
TNL_IMPLEMENT_NETOBJECT(RabbitFlagItem);

RabbitFlagItem::RabbitFlagItem(Point pos) : Item(pos, false, 20)
{
   mTeam = 0;
   mNetFlags.set(Ghostable);
   mObjectTypeMask |= CommandMapVisType;
}

void RabbitFlagItem::processArguments(S32 argc, const char **argv)
{
   if(argc < 4)
      return;

   mReturnTimer = Timer(atoi(argv[0]) * 1000);
   mScoreTimer = Timer(1.0f / atoi(argv[1]) * 60 * 1000); //secs per point
   Parent::processArguments(argc-2, argv+2);
   initialPos = mMoveState[ActualState].pos;
}

void RabbitFlagItem::onAddedToGame(Game *theGame)
{
   if(!isGhost())
      setScopeAlways();
}

void RabbitFlagItem::renderItem(Point pos)
{
   Point offset;

   if(mIsMounted)
      offset.set(15, -15);

   Color c;
   GameType *gt = getGame()->getGameType();

   c = gt->mTeams[getTeam()].color;

   renderRabbitFlag(pos + offset, c);
}

bool RabbitFlagItem::collide(GameObject *hitObject)
{
   if(mIsMounted)
      return false;

   if(!(hitObject->getObjectTypeMask() & ShipType))
      return true;

   if(isGhost() || ((Ship *) hitObject)->hasExploded)
      return false;

   RabbitGameType *theGameType = dynamic_cast<RabbitGameType *>(getGame()->getGameType());
   if(theGameType)
   {
      theGameType->onFlagGrabbed((Ship *) hitObject, this);
      return true;
   }
   return false;
}

void RabbitFlagItem::onMountDestroyed()
{
   RabbitGameType *game = dynamic_cast<RabbitGameType *>(getGame()->getGameType());
   if(!game)
      return;

   if(!mMount.isValid())
      return;

   mScoreTimer.reset();
   mReturnTimer.reset();

   game->onFlagDropped(this->mMount);
   Point vel = mMount->getActualVel();
   dismount();

   mMoveState[ActualState].vel = vel;
   setMaskBits(WarpPositionMask | PositionMask);
}

void RabbitFlagItem::sendHome()
{
   setActualPos(initialPos);

   RabbitGameType *game = dynamic_cast<RabbitGameType *>(getGame()->getGameType());
   if(!game)
      return;
   
   game->onFlagReturned();
}


void RabbitFlagItem::idle(GameObject::IdleCallPath path)
{
   U32 deltaT = mCurrentMove.time;

   if (mMount)
   {
      if (mScoreTimer.update(deltaT))
      {
         RabbitGameType *theGameType = dynamic_cast<RabbitGameType *>(getGame()->getGameType());
         if(!theGameType)
            return;

         theGameType->onFlagHeld(mMount);
         mScoreTimer.reset();
      }
   }
   else
   {
      if (!(initialPos == mMoveState[ActualState].pos) && mReturnTimer.update(deltaT))
      {
         mReturnTimer.reset();
         sendHome();
      }
   }
   Parent::idle(path);
}


};  //namespace Zap

