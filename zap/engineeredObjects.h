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

#ifndef _ENGINEEREDOBJECTS_H_
#define _ENGINEEREDOBJECTS_H_

#include "gameObject.h"
#include "item.h"
#include "barrier.h"

namespace Zap
{

extern void engClientCreateObject(GameConnection *connection, U32 object);

class EngineeredObject : public GameObject
{
   typedef GameObject Parent;
protected:
   F32 mHealth;
   S32 mTeam;
   Color mTeamColor;
   SafePtr<Item> mResource;
   SafePtr<Ship> mOwner;
   Point mAnchorPoint;
   Point mAnchorNormal;

   enum MaskBits
   {
      InitialMask  = BIT(0),
      NextFreeMask = BIT(1),
   };
   
public:
   EngineeredObject(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point());
   void setResource(Item *resource);
   bool checkDeploymentPosition();
   void computeExtent();
   virtual void onDestroyed();

   S32 getTeam() { return mTeam; }
   void setOwner(Ship *owner) { mOwner = owner; }

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   void damageObject(DamageInfo *damageInfo);
   bool collide(GameObject *hitObject) { return true; }
};

class ForceField : public GameObject
{
   Point mStart, mEnd;
   S32 mTeam;
   Timer mDownTimer;
   bool mFieldUp;

public:
   enum Constants
   {
      InitialMask = BIT(0),
      StatusMask = BIT(1),

      FieldDownTime = 1000,
   };

   ForceField(S32 team = -1, Point start = Point(), Point end = Point());
   bool collide(GameObject *hitObject);
   void idle(GameObject::IdleCallPath path);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   bool getCollisionPoly(Vector<Point> &polyPoints);
   void render();

   TNL_DECLARE_CLASS(ForceField);
};

class ForceFieldProjector : public EngineeredObject
{
   typedef EngineeredObject Parent;

   SafePtr<ForceField> mField;
public:
   ForceFieldProjector(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point()) :
      EngineeredObject(team, anchorPoint, anchorNormal) { mNetFlags.set(Ghostable); }

   void onDestroyed();
   void onAddedToGame(Game *theGame);

   bool getCollisionPoly(Vector<Point> &polyPoints);
   void render();
   TNL_DECLARE_CLASS(ForceFieldProjector);
};

class Turret : public EngineeredObject
{
   typedef EngineeredObject Parent;

   enum {
      TurretAimOffset = 25,
      TurretRange = 600,
      TurretPerceptionDistance = 800,
      TurretProjectileVelocity = 600,
      TurretTurnRate = 4,
      TurretFireDelay = 150,

      AimMask = EngineeredObject::NextFreeMask,
   };

   Timer mFireTimer;
   F32 mCurrentAngle;

public:
   Turret(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point(1, 0));

   bool getCollisionPoly(Vector<Point> &polyPoints);
   void render();
   void idle(IdleCallPath path);
   void onAddedToGame(Game *theGame);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   TNL_DECLARE_CLASS(Turret);
};

};

#endif