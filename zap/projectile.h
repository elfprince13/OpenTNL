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

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "gameObject.h"
#include "item.h"

namespace Zap
{

class Ship;

class Projectile : public GameObject
{
public:
   enum {
      CompressedVelocityMax = 2047,
      InitialMask = BIT(0),
      ExplodedMask = BIT(1),
   };

   enum ProjectileType
   {
      Phaser,
      Bounce,
      Triple,
      Turret,
      TypeCount,
   };

   Point pos;
   Point velocity;
   U32 liveTime;
   U32 mType;
   bool collided;
   bool alive;
   SafePtr<GameObject> mShooter;

   Projectile(U32 type = Phaser, Point pos = Point(), Point vel = Point(), U32 liveTime = 0, GameObject *shooter = NULL);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   void handleCollision(GameObject *theObject, Point collisionPoint);

   void idle(GameObject::IdleCallPath path);
   void explode(GameObject *hitObject, Point p);

   virtual Point getRenderVel() { return velocity; }
   virtual Point getActualVel() { return velocity; }

   void render();
   TNL_DECLARE_CLASS(Projectile);
};

/*
class Mine : public GameObject
{
public:
   Mine(Point pos, Ship *planter);

   GameType::ClientRef mOwner;

   void handleCollision(GameObject *theObject, Point colPoint);
   bool collide(GameObject *hitObject);

   void idle(IdleCallPath path);
   void render();
};
*/
class GrenadeProjectile : public Item
{
   typedef Item Parent;
public:
   GrenadeProjectile(Point pos = Point(), Point vel = Point(), U32 liveTime = 0, GameObject *shooter = NULL);

   enum Constants
   {
      ExplodeMask = BIT(8),
   };

   S32 ttl;
   bool exploded;

   bool collide(GameObject *otherObj) { return true; };

   void renderItem(Point p);
   void idle(IdleCallPath path);
   void damageObject(DamageInfo *damageInfo);
   void explode(Point p);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   TNL_DECLARE_CLASS(GrenadeProjectile);
};

};
#endif
