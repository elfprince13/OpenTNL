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

#include "ship.h"
#include "item.h"

#include "glutInclude.h"
#include "sparkManager.h"
#include "projectile.h"
#include "gameLoader.h"
#include "sfx.h"
#include "UI.h"
#include "UIMenus.h"
#include "UIGame.h"
#include "gameType.h"
#include "gameConnection.h"
#include "shipItems.h"
#include "gameWeapons.h"
#include <stdio.h>

namespace Zap
{

//------------------------------------------------------------------------
TNL_IMPLEMENT_NETOBJECT(Ship);

Ship::Ship(StringTableEntry playerName, S32 team, Point p, F32 m) : MoveObject(p, CollisionRadius)
{
   mObjectTypeMask = ShipType | MoveableType | CommandMapVisType;

   mNetFlags.set(Ghostable);

   for(U32 i = 0; i < MoveStateCount; i++)
   {
      mMoveState[i].pos = p;
      mMoveState[i].angle = 0;
   }
   mTeam = team;
   mHealth = 1.0;
   mass = m;
   hasExploded = false;
   updateExtent();

   mPlayerName = playerName;

   for(S32 i=0; i<TrailCount; i++)
      mTrail[i].reset();

   mEnergy = EnergyMax;
   for(S32 i = 0; i < ModuleCount; i++)
      mModuleActive[i] = false;

   mModule[0] = ModuleBoost;
   mModule[1] = ModuleShield;

   mWeapon[0] = WeaponPhaser;
   mWeapon[1] = WeaponMineLayer;
   mWeapon[2] = WeaponBurst;
   
   mActiveWeapon = 0;

   mCooldown = false;
}

void Ship::onGhostRemove()
{
   Parent::onGhostRemove();
   for(S32 i = 0; i < ModuleCount; i++)
      mModuleActive[i] = false;
   updateModuleSounds();
}

void Ship::processArguments(S32 argc, const char **argv)
{
   if(argc != 5)
      return;

   Point pos;
   pos.read(argv);
   pos *= getGame()->getGridSize();
   for(U32 i = 0; i < MoveStateCount; i++)
   {
      mMoveState[i].pos = pos;
      mMoveState[i].angle = 0;
   }

   color.read(argv + 2);
   updateExtent();
}

void Ship::setActualPos(Point p)
{
   mMoveState[ActualState].pos = p;
   mMoveState[RenderState].pos = p;
   setMaskBits(PositionMask | WarpPositionMask);
}

// process a move.  This will advance the position of the ship, as well as adjust the velocity and angle.
void Ship::processMove(U32 stateIndex)
{
   U32 msTime = mCurrentMove.time;

   mMoveState[LastProcessState] = mMoveState[stateIndex];

   F32 maxVel = isBoostActive() ? BoostMaxVelocity : MaxVelocity;

   F32 time = mCurrentMove.time * 0.001;
   Point requestVel(mCurrentMove.right - mCurrentMove.left, mCurrentMove.down - mCurrentMove.up);
   requestVel *= maxVel;
   F32 len = requestVel.len();

   if(len > maxVel)
      requestVel *= maxVel / len;

   Point velDelta = requestVel - mMoveState[stateIndex].vel;
   F32 accRequested = velDelta.len();

   F32 maxAccel = (isBoostActive() ? BoostAcceleration : Acceleration) * time;
   if(accRequested > maxAccel)
   {
      velDelta *= maxAccel / accRequested;
      mMoveState[stateIndex].vel += velDelta;
   }
   else
      mMoveState[stateIndex].vel = requestVel;

   mMoveState[stateIndex].angle = mCurrentMove.angle;
   move(time, stateIndex, false);
}

Point Ship::getAimVector()
{
   return Point(cos(mMoveState[ActualState].angle), sin(mMoveState[ActualState].angle) );
}

void Ship::selectWeapon()
{
   selectWeapon(mActiveWeapon + 1);
}

void Ship::selectWeapon(U32 weaponIdx)
{
   mActiveWeapon = weaponIdx % ShipWeaponCount;
   GameConnection *cc = getControllingClient();
   if(cc)
   {
      Vector<StringTableEntry> e;
      e.push_back(gWeapons[mWeapon[mActiveWeapon]].name);
      static StringTableEntry msg("%e0 selected.");
      cc->s2cDisplayMessageE(GameConnection::ColorAqua, SFXUIBoop, msg, e);
   }
}

void Ship::processWeaponFire()
{
   mFireTimer.update(mCurrentMove.time);

   U32 curWeapon = mWeapon[mActiveWeapon];

   if(mCurrentMove.fire && mFireTimer.getCurrent() == 0)
   {
      if(mEnergy >= gWeapons[curWeapon].minEnergy)
      {
         mEnergy -= gWeapons[curWeapon].drainEnergy;
         mFireTimer.reset(gWeapons[curWeapon].fireDelay);

         if(!isGhost())
         {
            Point dir = getAimVector();
            createWeaponProjectiles(curWeapon, dir, mMoveState[ActualState].pos, mMoveState[ActualState].vel, CollisionRadius - 2, this);
         }
      }
   }
}

void Ship::controlMoveReplayComplete()
{
   // compute the delta between our current render position
   // and the server position after client-side prediction has
   // been run.
   Point delta = mMoveState[ActualState].pos - mMoveState[RenderState].pos;
   F32 deltaLen = delta.len();

   // if the delta is either very small, or greater than the
   // max interpolation threshold, just warp to the new position
   if(deltaLen <= 0.5 || deltaLen > MaxControlObjectInterpDistance)
   {
      // if it's a large delta, get rid of the movement trails.
      if(deltaLen > MaxControlObjectInterpDistance)
         for(S32 i=0; i<TrailCount; i++)
            mTrail[i].reset();

      mMoveState[RenderState].pos = mMoveState[ActualState].pos;
      mMoveState[RenderState].vel = mMoveState[ActualState].vel;
      mInterpolating = false;
   }
   else
      mInterpolating = true;
}

void Ship::idle(GameObject::IdleCallPath path)
{
   // don't process exploded ships
   if(hasExploded)
      return;

   if(path == GameObject::ServerIdleMainLoop && isControlled())
   {
      // if this is a controlled object in the server's main
      // idle loop, process the render state forward -- this
      // is what projectiles will collide against.  This allows
      // clients to properly lead other clients, instead of
      // piecewise stepping only when packets arrive from the client.
      processMove(RenderState);
      setMaskBits(PositionMask);
   }
   else
   {
      // for all other cases, advance the actual state of the
      // object with the current move.
      processMove(ActualState);

      if(path == GameObject::ServerIdleControlFromClient ||
         path == GameObject::ClientIdleControlMain ||
         path == GameObject::ClientIdleControlReplay)
      {
         // for different optimizer settings and different platforms
         // the floating point calculations may come out slightly
         // differently in the lowest mantissa bits.  So normalize
         // after each update the position and velocity, so that
         // the control state update will not differ from client to server.
         const F32 ShipVarNormalizeMultiplier = 128;
         const F32 ShipVarNormalizeFraction = 1 / ShipVarNormalizeMultiplier;

         mMoveState[ActualState].pos.scaleFloorDiv(ShipVarNormalizeMultiplier, ShipVarNormalizeFraction);
         mMoveState[ActualState].vel.scaleFloorDiv(ShipVarNormalizeMultiplier, ShipVarNormalizeFraction);
      }

      if(path == GameObject::ServerIdleMainLoop || 
         path == GameObject::ServerIdleControlFromClient)
      {
         // update the render state on the server to match
         // the actual updated state, and mark the object
         // as having changed Position state.  An optimization
         // here would check the before and after positions
         // so as to not update unmoving ships.
         mMoveState[RenderState] = mMoveState[ActualState];
         setMaskBits(PositionMask);
      }
      else if(path == GameObject::ClientIdleControlMain ||
              path == GameObject::ClientIdleMainRemote)
      {
         // on the client, update the interpolation of this object
         // only if we are not replaying control moves.
         mInterpolating = (getActualVel().lenSquared() < MoveObject::InterpMaxVelocity*MoveObject::InterpMaxVelocity);
         updateInterpolation();
      }
   }

   // update the object in the game's extents database.
   updateExtent();

   // if this is a move executing on the server and it's
   // different from the last move, then mark the move to
   // be updated to the ghosts.
   if(path == GameObject::ServerIdleControlFromClient && 
         !mCurrentMove.isEqualMove(&mLastMove))
      setMaskBits(MoveMask);

   mLastMove = mCurrentMove;
   mSensorZoomTimer.update(mCurrentMove.time);
   mCloakTimer.update(mCurrentMove.time);

   bool engineerWasActive = isEngineerActive();

   if(path == GameObject::ServerIdleControlFromClient ||
      path == GameObject::ClientIdleControlMain ||
      path == GameObject::ClientIdleControlReplay)
   {
      // process weapons and energy on controlled object objects
      processWeaponFire();
      processEnergy();
   }

   // if we're on the client and the user has just activated
   // the engineering pack, notify the game user interface
   // that it should display the buildable items list.
   if(path == GameObject::ClientIdleControlMain &&
      !engineerWasActive && isEngineerActive() &&
      carryingResource())
   {
      // it's kind of gross to have this here... but this
      // module behaves in a pretty non-standard way.
      gGameUserInterface.displayEngineerBuildMenu();
   }
   
   if(path == GameObject::ClientIdleMainRemote)
   {
      // for ghosts, find some repair targets for rendering the
      // repair effect.
      if(isRepairActive())
         findRepairTargets();
   }
   if(path == GameObject::ServerIdleControlFromClient &&
      isRepairActive())
   {
      repairTargets();
   }

   if(path == GameObject::ClientIdleControlMain ||
      path == GameObject::ClientIdleMainRemote)
   {
      // Emit some particles, trail sections and update the turbo noise
      emitMovementSparks();
      for(U32 i=0; i<TrailCount; i++)
         mTrail[i].tick(mCurrentMove.time);
      updateModuleSounds();
   }
}

bool Ship::findRepairTargets()
{
   Vector<GameObject *> hitObjects;
   Point pos = getActualPos();
   Point extend(RepairRadius, RepairRadius);
   Rect r(pos - extend, pos + extend);
   findObjects(ShipType | EngineeredType, hitObjects, r);

   mRepairTargets.clear();
   for(S32 i = 0; i < hitObjects.size(); i++)
   {
      GameObject *s = hitObjects[i];
      if(!s->isDestroyed() && s->getHealth() < 1 && (s->getActualPos() - pos).len() < (RepairRadius + CollisionRadius))
         mRepairTargets.push_back(s);
   }
   return mRepairTargets.size() != 0;
}

void Ship::repairTargets()
{
   F32 totalRepair = RepairHundredthsPerSecond * 0.01 * mCurrentMove.time * 0.001f;


//   totalRepair /= mRepairTargets.size();

   DamageInfo di;
   di.damageAmount = -totalRepair;
   di.damagingObject = this;
   di.damageType = 0;

   for(S32 i = 0; i < mRepairTargets.size(); i++)
      mRepairTargets[i]->damageObject(&di);
}

void Ship::processEnergy()
{
   static U32 gEnergyDrain[ModuleCount] = 
   {
      Ship::EnergyShieldDrain,
      Ship::EnergyBoostDrain,
      Ship::EnergySensorDrain,
      Ship::EnergyRepairDrain,
      0,
      Ship::EnergyCloakDrain,
   };

   bool modActive[ModuleCount];
   for(S32 i = 0; i < ModuleCount; i++)
   {
      modActive[i] = mModuleActive[i];
      mModuleActive[i] = false;
   }

   if(mEnergy > EnergyCooldownThreshold)
      mCooldown = false;

   for(S32 i = 0; i < ShipModuleCount; i++)
      if(mCurrentMove.module[i] && !mCooldown)
         mModuleActive[mModule[i]] = true;

    if(mModuleActive[ModuleBoost] && 
       mCurrentMove.up == 0 && 
       mCurrentMove.down == 0 && 
       mCurrentMove.left == 0 && 
       mCurrentMove.right == 0) 
   { 
      mModuleActive[ModuleBoost] = false; 
   }
   if(mModuleActive[ModuleRepair] && !findRepairTargets())
      mModuleActive[ModuleRepair] = false;

   F32 scaleFactor = mCurrentMove.time * 0.001;

   // Update things based on available energy...
   bool anyActive = false;
   for(S32 i = 0; i < ModuleCount; i++)
   {
      if(mModuleActive[i])
      {
         mEnergy -= S32(gEnergyDrain[i] * scaleFactor);
         anyActive = true;
      }
   }

   if(!anyActive && mEnergy <= EnergyCooldownThreshold)
      mCooldown = true;

   if(mEnergy < EnergyMax)
   {
      // If we're not doing anything, recharge.
      if(!anyActive)
         mEnergy += S32(EnergyRechargeRate * scaleFactor);

      if(mEnergy <= 0)
      {
         mEnergy = 0;
         for(S32 i = 0; i < ModuleCount; i++)
            mModuleActive[i] = false;
         mCooldown = true;
      }
   }

   if(mEnergy >= EnergyMax)
      mEnergy = EnergyMax;

   for(S32 i = 0; i < ModuleCount;i++)
   {
      if(mModuleActive[i] != modActive[i])
      {
         if(i == ModuleSensor)
            mSensorZoomTimer.reset(SensorZoomTime - mSensorZoomTimer.getCurrent(), SensorZoomTime);
         else if(i == ModuleCloak)
            mCloakTimer.reset(CloakFadeTime - mCloakTimer.getCurrent(), CloakFadeTime);
         setMaskBits(PowersMask);
         break;
      }
   }
}

void Ship::damageObject(DamageInfo *theInfo)
{
   if(theInfo->damageAmount > 0)
   {
      if(!getGame()->getGameType()->objectCanDamageObject(theInfo->damagingObject, this))
         return;

      // Factor in shields
      if(isShieldActive() && mEnergy >= EnergyShieldHitDrain)
      {
         mEnergy -= EnergyShieldHitDrain;
         return;
      }
   }
   mHealth -= theInfo->damageAmount;
   setMaskBits(HealthMask);
   if(mHealth <= 0)
   {
      mHealth = 0;
      kill(theInfo);
   }
   else if(mHealth > 1)
      mHealth = 1;

   // Deal with grenades
   if(theInfo->damageType == 1)
   {
      mMoveState[0].vel += theInfo->impulseVector;
   }
}


void Ship::updateModuleSounds()
{
   static S32 moduleSFXs[ModuleCount] = 
   {
      -1,
      SFXShipBoost,
      SFXShipBoost,
      -1,
      -1,
      SFXShipBoost,
   };

   for(U32 i = 0; i < ModuleCount; i++)
   {
      if(mModuleActive[i])
      {
         if(mModuleSound[i].isValid())
            mModuleSound[i]->setMovementParams(mMoveState[RenderState].pos, mMoveState[RenderState].vel);
         else if(moduleSFXs[i] != -1)
            mModuleSound[i] = SFXObject::play(moduleSFXs[i], mMoveState[RenderState].pos, mMoveState[RenderState].vel);
      }
      else
      {
         if(mModuleSound[i].isValid())
         {
            mModuleSound[i]->stop();
            mModuleSound[i] = 0;
         }
      }
   }
}


void Ship::writeControlState(BitStream *stream)
{
   stream->write(mMoveState[ActualState].pos.x);
   stream->write(mMoveState[ActualState].pos.y);
   stream->write(mMoveState[ActualState].vel.x);
   stream->write(mMoveState[ActualState].vel.y);
   stream->writeRangedU32(mEnergy, 0, EnergyMax);
   stream->writeFlag(mCooldown);
   stream->writeRangedU32(mFireTimer.getCurrent(), 0, MaxFireDelay);
   stream->writeRangedU32(mActiveWeapon, 0, WeaponCount);
}

void Ship::readControlState(BitStream *stream)
{
   stream->read(&mMoveState[ActualState].pos.x);
   stream->read(&mMoveState[ActualState].pos.y);
   stream->read(&mMoveState[ActualState].vel.x);
   stream->read(&mMoveState[ActualState].vel.y);
   mEnergy = stream->readRangedU32(0, EnergyMax);
   mCooldown = stream->readFlag();
   U32 fireTimer = stream->readRangedU32(0, MaxFireDelay);
   mFireTimer.reset(fireTimer);
   mActiveWeapon = stream->readRangedU32(0, WeaponCount);
}

U32  Ship::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   GameConnection *gameConnection = (GameConnection *) connection;

   if(stream->writeFlag(updateMask & InitialMask))
   {
      connection->packStringTableEntry(stream, mPlayerName);
      stream->write(mass);
      stream->writeRangedU32(mTeam + 1, 0, getGame()->getTeamCount());

      // now write all the mounts:
      for(S32 i = 0; i < mMountedItems.size(); i++)
      {
         if(mMountedItems[i].isValid())
         {
            S32 index = connection->getGhostIndex(mMountedItems[i]);
            if(index != -1)
            {
               stream->writeFlag(true);
               stream->writeInt(index, GhostConnection::GhostIdBitSize);
            }
         }
      }
      stream->writeFlag(false);
   }
   if(stream->writeFlag(updateMask & HealthMask))
      stream->writeFloat(mHealth, 6);

   if(stream->writeFlag(updateMask & LoadoutMask))
   {
      stream->writeRangedU32(mModule[0], 0, ModuleCount);
      stream->writeRangedU32(mModule[1], 0, ModuleCount);
   }

   stream->writeFlag(hasExploded);

   bool shouldWritePosition = (updateMask & InitialMask) || 
      gameConnection->getControlObject() != this;

   if(!shouldWritePosition)
   {
      stream->writeFlag(false);
      stream->writeFlag(false);
      stream->writeFlag(false);
   }
   else
   {
      if(stream->writeFlag(updateMask & PositionMask))
      {
         gameConnection->writeCompressedPoint(mMoveState[RenderState].pos, stream);
         writeCompressedVelocity(mMoveState[RenderState].vel, BoostMaxVelocity + 1, stream);
         stream->writeFlag(updateMask & WarpPositionMask);
      }
      if(stream->writeFlag(updateMask & MoveMask))
      {
         mCurrentMove.pack(stream, NULL, false);
      }
      if(stream->writeFlag(updateMask & PowersMask))
      {
         for(S32 i = 0; i < ModuleCount; i++)
            stream->writeFlag(mModuleActive[i]);
      }
   }
   return 0;
}

void Ship::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   bool interpolate = false;
   bool positionChanged = false;
   bool wasInitialUpdate = false;

   if(stream->readFlag())
   {
      wasInitialUpdate = true;

      mPlayerName = connection->unpackStringTableEntry(stream);
      GameType *g = gClientGame->getGameType();
      if(g)
         color = g->getClientColor(mPlayerName);

      stream->read(&mass);
      mTeam = stream->readRangedU32(0, gClientGame->getTeamCount()) - 1;

      // read mounted items:
      while(stream->readFlag())
      {
         S32 index = stream->readInt(GhostConnection::GhostIdBitSize);
         Item *theItem = (Item *) connection->resolveGhost(index);
         theItem->mountToShip(this);
      }
   }

   if(stream->readFlag())
      mHealth = stream->readFloat(6);

   if(stream->readFlag())
   {
      mModule[0] = stream->readRangedU32(0, ModuleCount);
      mModule[1] = stream->readRangedU32(0, ModuleCount);
   }

   bool explode = stream->readFlag();

   if(stream->readFlag())
   {
      ((GameConnection *) connection)->readCompressedPoint(mMoveState[ActualState].pos, stream);
      readCompressedVelocity(mMoveState[ActualState].vel, BoostMaxVelocity + 1, stream);
      positionChanged = true;
      interpolate = !stream->readFlag();
   }
   if(stream->readFlag())
   {
      mCurrentMove = Move();
      mCurrentMove.unpack(stream, false);
   }
   if(stream->readFlag())
   {
      bool wasActive[ModuleCount];
      for(S32 i = 0; i < ModuleCount; i++)
      {
         wasActive[i] = mModuleActive[i];
         mModuleActive[i] = stream->readFlag();
         if(i == ModuleSensor && wasActive[i] != mModuleActive[i])
            mSensorZoomTimer.reset(SensorZoomTime - mSensorZoomTimer.getCurrent(), SensorZoomTime);
         if(i == ModuleCloak && wasActive[i] != mModuleActive[i])
            mCloakTimer.reset(CloakFadeTime - mCloakTimer.getCurrent(), CloakFadeTime);
      }
   }

   mMoveState[ActualState].angle = mCurrentMove.angle;

   if(positionChanged)
   {
      mCurrentMove.time = (U32) connection->getOneWayTime();
      processMove(ActualState);

      if(interpolate)
      {
         mInterpolating = true;
         // if the actual velocity is in the direction of the actual position
         // then we'll set it into the render velocity
      }
      else
      {
         mInterpolating = false;
         mMoveState[RenderState] = mMoveState[ActualState];

         for(S32 i=0; i<TrailCount; i++)
            mTrail[i].reset();
      }
   }
   if(explode && !hasExploded)
   {
      hasExploded = true;
      disableCollision();

      if(!wasInitialUpdate)
         emitShipExplosion(mMoveState[ActualState].pos);
   }
}

static Vector<GameObject *> fillVector;

F32 getAngleDiff(F32 a, F32 b)
{
   // Figure out the shortest path from a to b...
   // Restrict them to the range 0-360
   while(a<0)   a+=360;
   while(a>360) a-=360;

   while(b<0)   b+=360;
   while(b>360) b-=360;

   if(fabs(b-a) > 180)
   {
      // Go the other way
      return  360-(b-a);
   }
   else
   {
      return b-a;
   }
}

bool Ship::carryingResource()
{
   for(S32 i = mMountedItems.size() - 1; i >= 0; i--)
      if(mMountedItems[i].isValid() && mMountedItems[i]->getObjectTypeMask() & ResourceItemType)
         return true;
   return false;
}

Item *Ship::unmountResource()
{
   for(S32 i = mMountedItems.size() - 1; i >= 0; i--)
   {
      if(mMountedItems[i]->getObjectTypeMask() & ResourceItemType)
      {
         Item *ret = mMountedItems[i];
         ret->dismount();
         return ret;
      }
   }
   return NULL;
}

void Ship::setLoadout(U32 module1, U32 module2, U32 weapon1, U32 weapon2, U32 weapon3)
{
   if(module1 == mModule[0] && module2 == mModule[1]
      && weapon1 == mWeapon[0] && weapon2 == mWeapon[1] && weapon3 == mWeapon[2])
         return;

   U32 currentWeapon = mWeapon[mActiveWeapon];

   mModule[0] = module1;
   mModule[1] = module2; 
   mWeapon[0] = weapon1;
   mWeapon[1] = weapon2;
   mWeapon[2] = weapon3;

   setMaskBits(LoadoutMask);

   GameConnection *cc = getControllingClient();

   if(cc)
   {
      static StringTableEntry msg("Ship loadout configuration updated.");
      cc->s2cDisplayMessage(GameConnection::ColorAqua, SFXUIBoop, msg);
   }
   U32 i;
   for(i = 0; i < 3; i++)
   {
      if(mWeapon[i] == currentWeapon)
      {
         mActiveWeapon = i;
         break;
      }
   }
   if(i == 3)
      selectWeapon(0);

   if(mModule[0] != ModuleEngineer && mModule[1] != ModuleEngineer)
   {
      // drop any resources we may be carrying
      for(S32 i = mMountedItems.size() - 1; i >= 0; i--)
         if(mMountedItems[i]->getObjectTypeMask() & ResourceItemType)
            mMountedItems[i]->dismount();
   }
}

void Ship::kill()
{
   deleteObject(KillDeleteDelay);
   hasExploded = true;
   setMaskBits(ExplosionMask);
   disableCollision();
   for(S32 i = mMountedItems.size() - 1; i >= 0; i--)
      mMountedItems[i]->onMountDestroyed();
}

void Ship::kill(DamageInfo *theInfo)
{
   if(isGhost())
      return;

   GameConnection *controllingClient = getControllingClient();
   if(controllingClient)
   {
      GameType *gt = getGame()->getGameType();
      if(gt)
         gt->controlObjectForClientKilled(controllingClient, this, theInfo->damagingObject);
   }
   kill();
}

enum {
   NumShipExplosionColors = 12,
};

Color ShipExplosionColors[NumShipExplosionColors] = {
Color(1, 0, 0),
Color(0.9, 0.5, 0),
Color(1, 1, 1),
Color(1, 1, 0),
Color(1, 0, 0),
Color(0.8, 1.0, 0),
Color(1, 0.5, 0),
Color(1, 1, 1),
Color(1, 0, 0),
Color(0.9, 0.5, 0),
Color(1, 1, 1),
Color(1, 1, 0),
};

void Ship::emitShipExplosion(Point pos)
{
   SFXObject::play(SFXShipExplode, pos, Point());

   F32 a, b;
   a = Random::readF() * 0.4 + 0.5;
   b = Random::readF() * 0.2 + 0.9;

   F32 c, d;
   c = Random::readF() * 0.15 + 0.125;
   d = Random::readF() * 0.2 + 0.9;

   SparkManager::emitExplosion(mMoveState[ActualState].pos, 0.9, ShipExplosionColors, NumShipExplosionColors);
   SparkManager::emitBurst(pos, Point(a,c), Color(1,1,0.25), Color(1,0,0));
   SparkManager::emitBurst(pos, Point(b,d), Color(1,1,0), Color(0,0.75,0));
}

void Ship::emitMovementSparks()
{
   U32 deltaT = mCurrentMove.time;

   // Do nothing if we're under 0.1 vel
   if(hasExploded || mMoveState[ActualState].vel.len() < 0.1)
      return;

   mSparkElapsed += deltaT;

   if(mSparkElapsed <= 32)
      return;

   bool boostActive = isBoostActive();
   bool cloakActive = isCloakActive();

   Point corners[3];
   Point shipDirs[3];

   corners[0].set(-20, -15);
   corners[1].set(  0,  25);
   corners[2].set( 20, -15);

   F32 th = FloatHalfPi - mMoveState[RenderState].angle;

   F32 sinTh = sin(th);
   F32 cosTh = cos(th);

   for(S32 i=0; i<3; i++)
   {
      shipDirs[i].x = corners[i].x * cosTh + corners[i].y * sinTh;
      shipDirs[i].y = corners[i].y * cosTh - corners[i].x * sinTh;
   }

   Point leftVec ( mMoveState[ActualState].vel.y, -mMoveState[ActualState].vel.x);
   Point rightVec(-mMoveState[ActualState].vel.y,  mMoveState[ActualState].vel.x);

   leftVec.normalize();
   rightVec.normalize();

   S32 bestId = -1, leftId, rightId;
   F32 bestDot = -1;

   // Find the left-wards match
   for(S32 i = 0; i < 3; i++)
   {
      F32 d = leftVec.dot(shipDirs[i]);
      if(d >= bestDot)
      {
         bestDot = d;
         bestId = i;
      }
   }

   leftId = bestId;
   Point leftPt = mMoveState[RenderState].pos + shipDirs[bestId];

   // Find the right-wards match
   bestId = -1;
   bestDot = -1;
   
   for(S32 i = 0; i < 3; i++)
   {
      F32 d = rightVec.dot(shipDirs[i]);
      if(d >= bestDot)
      {
         bestDot = d;
         bestId = i;
      }      
   }

   rightId = bestId;
   Point rightPt = mMoveState[RenderState].pos + shipDirs[bestId];

   // Stitch things up if we must...
   if(leftId == mLastTrailPoint[0] && rightId == mLastTrailPoint[1])
   {
      mTrail[0].update(leftPt,  boostActive, cloakActive);
      mTrail[1].update(rightPt, boostActive, cloakActive); 
      mLastTrailPoint[0] = leftId;
      mLastTrailPoint[1] = rightId;
   }
   else if(leftId == mLastTrailPoint[1] && rightId == mLastTrailPoint[0])
   {
      mTrail[1].update(leftPt,  boostActive, cloakActive);
      mTrail[0].update(rightPt, boostActive, cloakActive);
      mLastTrailPoint[1] = leftId;
      mLastTrailPoint[0] = rightId;
   }
   else
   {
      mTrail[0].update(leftPt,  boostActive, cloakActive);
      mTrail[1].update(rightPt, boostActive, cloakActive);
      mLastTrailPoint[0] = leftId;
      mLastTrailPoint[1] = rightId;
   }

   if(isCloakActive())
      return;

   // Finally, do some particles
   Point velDir(mCurrentMove.right - mCurrentMove.left, mCurrentMove.down - mCurrentMove.up);
   F32 len = velDir.len();

   if(len > 0)
   {
      if(len > 1)
         velDir *= 1 / len;

      Point shipDirs[4];
      shipDirs[0].set(cos(mMoveState[RenderState].angle), sin(mMoveState[RenderState].angle) );
      shipDirs[1].set(-shipDirs[0]);
      shipDirs[2].set(shipDirs[0].y, -shipDirs[0].x);
      shipDirs[3].set(-shipDirs[0].y, shipDirs[0].x);

      for(U32 i = 0; i < 4; i++)
      {
         F32 th = shipDirs[i].dot(velDir);

          if(th > 0.1)
          {
             // shoot some sparks...
             if(th >= 0.2*velDir.len())
             {
                Point chaos(TNL::Random::readF(),TNL::Random::readF());
                chaos *= 5;
 
                //interp give us some nice enginey colors...
                Color dim(1, 0, 0);
                Color light(1, 1, boostActive ? 1.f : 0.f);
                Color thrust;
  
                F32 t = TNL::Random::readF();
                thrust.interp(t, dim, light);
  
                SparkManager::emitSpark(
                                        mMoveState[RenderState].pos - shipDirs[i] * 13,
                                        -shipDirs[i] * 100 + chaos,
                                        thrust,
                                        1.5 * Random::readF()
                                       );
             }
          }
      }
   }

}

void Ship::render()
{
   if(hasExploded)
      return;

   if(posSegments.size())
   {
      glBegin(GL_LINES);
      glColor3f(0,1,1);
      for(S32 i = 0; i < posSegments.size() - 1; i++)
      {
         glVertex2f(posSegments[i].x, posSegments[i].y);
         glVertex2f(posSegments[i+1].x, posSegments[i+1].y);
      }
      glEnd();
   }

   glPushMatrix();
   glTranslatef(mMoveState[RenderState].pos.x, mMoveState[RenderState].pos.y, 0);

   F32 alpha = 1.0;
   if(isCloakActive())
      alpha = 1 - mCloakTimer.getFraction();
   else
      alpha = mCloakTimer.getFraction();

   // Render name...
   GameConnection *conn = gClientGame->getConnectionToServer();
   if(conn && conn->getControlObject() != this)
   {
      static char buff[255];
      sprintf(buff, "%s", mPlayerName.getString());

      // Make it a nice pastel
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1,1,1,0.5 * alpha);
      //glColor3f(color.r*1.2,color.g*1.2,color.b*1.2);
      UserInterface::drawString( U32( UserInterface::getStringWidth(14, buff) * -0.5), 30, 14, buff );
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
   }
   
   if(alpha != 1.0)
   {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }

   // draw thrusters
   Point velDir(mCurrentMove.right - mCurrentMove.left, mCurrentMove.down - mCurrentMove.up);
   F32 len = velDir.len();
   F32 thrusts[4];
   for(U32 i = 0; i < 4; i++)
      thrusts[i] = 0;

   if(len > 0)
   {
      if(len > 1)
         velDir *= 1 / len;

      Point shipDirs[4];
      shipDirs[0].set(cos(mMoveState[RenderState].angle), sin(mMoveState[RenderState].angle) );
      shipDirs[1].set(-shipDirs[0]);
      shipDirs[2].set(shipDirs[0].y, -shipDirs[0].x);
      shipDirs[3].set(-shipDirs[0].y, shipDirs[0].x);

      for(U32 i = 0; i < 4; i++)
         thrusts[i] = shipDirs[i].dot(velDir);
   }

   // Tweak side thrusters to show rotational force
   F32 rotVel = getAngleDiff(mMoveState[LastProcessState].angle, mMoveState[RenderState].angle);

   if(rotVel > 0.001)
      thrusts[2] += 0.25;
   else if(rotVel < -0.001)
      thrusts[3] += 0.25;

   if(isBoostActive())
   {
      for(U32 i = 0; i < 4; i++)
         thrusts[i] *= 1.3;
   }

   // first render the thrusters

   // an angle of 0 means the ship is heading down the +X axis
   // since we draw the ship pointing up the Y axis, we should rotate
   // by the ship's angle, - 90 degrees
   glRotatef(radiansToDegrees(mMoveState[RenderState].angle) - 90, 0, 0, 1.0);
   
   if(isCloakActive())
   {
      glColor4f(0,0,0, 1 - alpha);
      glBegin(GL_POLYGON);
      glVertex2f(-20, -15);
      glVertex2f(0, 25);
      glVertex2f(20, -15);
      glEnd();
   }

   if(thrusts[0] > 0) // forward thrust:
   {
      glColor4f(1, 0, 0, alpha);
      glBegin(GL_LINES);
      glVertex2f(-8, -15);
      glVertex2f(0, -15 - 20 * thrusts[0]);
      glVertex2f(0, -15 - 20 * thrusts[0]);
      glVertex2f(8, -15);
      glEnd();
      glColor4f(1, 0.5, 0, alpha);
      glBegin(GL_LINES);
      glVertex2f(-6, -15);
      glVertex2f(0, -15 - 15 * thrusts[0]);
      glVertex2f(0, -15 - 15 * thrusts[0]);
      glVertex2f(6, -15);
      glEnd();
      glColor4f(1, 1, 0, alpha);
      glBegin(GL_LINES);
      glVertex2f(-4, -15);
      glVertex2f(0, -15 - 8 * thrusts[0]);
      glVertex2f(0, -15 - 8 * thrusts[0]);
      glVertex2f(4, -15);
      glEnd();
   }
   if(thrusts[1] > 0) // back thrust
   {
      // two jets:
      // left and right side:
      // from 7.5, 10 -> 12.5, 10 and from -7.5, 10 to -12.5, 10
      glColor4f(1, 0.5, 0, alpha);
      glBegin(GL_LINES);
      glVertex2f(7.5, 10);
      glVertex2f(10, 10 + thrusts[1] * 15);
      glVertex2f(12.5, 10);
      glVertex2f(10, 10 + thrusts[1] * 15);
      glVertex2f(-7.5, 10);
      glVertex2f(-10, 10 + thrusts[1] * 15);
      glVertex2f(-12.5, 10);
      glVertex2f(-10, 10 + thrusts[1] * 15);
      glEnd();
      glColor4f(1,1,0, alpha);
      glBegin(GL_LINES);
      glVertex2f(9, 10);
      glVertex2f(10, 10 + thrusts[1] * 10);
      glVertex2f(11, 10);
      glVertex2f(10, 10 + thrusts[1] * 10);
      glVertex2f(-9, 10);
      glVertex2f(-10, 10 + thrusts[1] * 10);
      glVertex2f(-11, 10);
      glVertex2f(-10, 10 + thrusts[1] * 10);
      glEnd();

   }
   float xThrust = -12.5;
   if(thrusts[3] > 0)
   {
      xThrust = -xThrust;
      thrusts[2] = thrusts[3];
   }
   if(thrusts[2] > 0)
   {
      glColor4f(1, 0, 0, alpha);
      glBegin(GL_LINES);
      glVertex2f(xThrust, 10);
      glVertex2f(xThrust + thrusts[2] * xThrust * 1.5, 5);
      glVertex2f(xThrust, 0);
      glVertex2f(xThrust + thrusts[2] * xThrust * 1.5, 5);
      glEnd();
      glColor4f(1,0.5,0, alpha);
      glBegin(GL_LINES);
      glVertex2f(xThrust, 8);
      glVertex2f(xThrust + thrusts[2] * xThrust, 5);
      glVertex2f(xThrust, 2);
      glVertex2f(xThrust + thrusts[2] * xThrust, 5);
      glEnd();
      glColor4f(1,1,0, alpha);
      glBegin(GL_LINES);
      glVertex2f(xThrust, 6);
      glVertex2f(xThrust + thrusts[2] * xThrust * 0.5, 5);
      glVertex2f(xThrust, 4);
      glVertex2f(xThrust + thrusts[2] * xThrust * 0.5, 5);
      glEnd();
   }

   // then render the ship:
   glColor4f(0.5,0.5,0.5, alpha);
   glBegin(GL_LINES);
   glVertex2f(-12.5, 0);
   glVertex2f(-12.5, 10);
   glVertex2f(-12.5, 10);
   glVertex2f(-7.5, 10);
   glVertex2f(7.5, 10);
   glVertex2f(12.5, 10);
   glVertex2f(12.5, 10);
   glVertex2f(12.5, 0);
   glEnd();
   
   glColor4f(color.r,color.g,color.b, alpha);
   glBegin(GL_LINE_LOOP);
   glVertex2f(-12, -13);
   glVertex2f(0, 22);
   glVertex2f(12, -13);
   glEnd();

   U32 health = U32(14 * mHealth);
   glBegin(GL_LINES);
   for(U32 i = 0; i < health; i++)
   {
      S32 yo = i * 2;
      glVertex2f(-2, -11 + yo);
      glVertex2f(2, -11 + yo);
   }
   glEnd();

   glColor4f(1,1,1, alpha);
   glBegin(GL_LINE_LOOP);
   glVertex2f(-20, -15);
   glVertex2f(0, 25);
   glVertex2f(20, -15);
   glEnd();

   // Render shield if appropriate
   if(isShieldActive())
   {
      F32 shieldRadius = mRadius + 3;

      glColor4f(1,1,0, alpha);
      glBegin(GL_LINE_LOOP);
      for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.3)
         glVertex2f(cos(theta) * shieldRadius, sin(theta) * shieldRadius);
      
      glEnd();
   }

   if(hasExploded)
   {
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
   }


   glPopMatrix();

   if(alpha != 1.0)
   {
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
   }

   for(S32 i = 0; i < mMountedItems.size(); i++)
      if(mMountedItems[i].isValid())
         mMountedItems[i]->renderItem(mMoveState[RenderState].pos);
 
   if(isRepairActive())
   {
      glLineWidth(3);
      glColor3f(1,0,0);
      // render repair rays to all the repairing objects
      Point pos = mMoveState[RenderState].pos;

      for(S32 i = 0; i < mRepairTargets.size(); i++)
      {
         if(mRepairTargets[i].getPointer() == this)
         {
            glBegin(GL_LINE_LOOP);
            for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.3)
               glVertex2f(pos.x + cos(theta) * RepairDisplayRadius, 
                          pos.y + sin(theta) * RepairDisplayRadius);
            
            glEnd();
         }
         else if(mRepairTargets[i])
         {
            glBegin(GL_LINES);
            glVertex2f(pos.x, pos.y);
            Point shipPos = mRepairTargets[i]->getRenderPos();
            glVertex2f(shipPos.x, shipPos.y);
            glEnd();
         }
      }
      glLineWidth(1);
   }
}

};
