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

#include "projectile.h"
#include "ship.h"
#include "sparkManager.h"
#include "sfx.h"

#include "glutInclude.h"

namespace Zap
{

enum {
   NumSparkColors = 4,
};

struct ProjectileInfo
{
   F32 damageAmount;
   Color sparkColors[NumSparkColors];
   Color projColors[2];
   F32 scaleFactor;
};

ProjectileInfo gProjInfo[Projectile::TypeCount] = {
   { 0.21f, { Color(1,0,1), Color(1,1,1), Color(0,0,1), Color(1,0,0) }, { Color(1, 0, 0.5), Color(0.5, 0, 1) }, 1 },
   { 0.15f, { Color(1,1,0), Color(1,0,0), Color(1,0.5,0), Color(1,1,1) }, { Color(1, 1, 0), Color(1, 0, 0) }, 1.3 },
   { 0.14f, { Color(0,0,1), Color(0,1,0), Color(0,0.5,1), Color(0,1,0.5) }, { Color(0, 0.5, 1), Color(0, 1, 0.5) }, 0.7 },
   { 0.11f, { Color(0,1,1), Color(1,1,0), Color(0,1,0.5), Color(0.5,1,0) }, { Color(0.5, 1, 0), Color(0, 1, 0.5) }, 0.6 },
};


TNL_IMPLEMENT_NETOBJECT(Projectile);

Projectile::Projectile(U32 type, Point p, Point v, U32 t, GameObject *shooter)
{
   mNetFlags.set(Ghostable);
   pos = p;
   velocity = v;
   mTimeRemaining = t;
   mAliveTime = 0;
   collided = false;
   alive = true;
   mShooter = shooter;
   mType = type;
}

U32 Projectile::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   if(stream->writeFlag(updateMask & InitialMask))
   {
      ((GameConnection *) connection)->writeCompressedPoint(pos, stream);
      writeCompressedVelocity(velocity, CompressedVelocityMax, stream);

      stream->writeEnum(mType, TypeCount);

      S32 index = -1;
      if(mShooter.isValid())
         index = connection->getGhostIndex(mShooter);
      if(stream->writeFlag(index != -1))
         stream->writeInt(index, GhostConnection::GhostIdBitSize);
   }
   stream->writeFlag(collided);
   stream->writeFlag(alive);
   return 0;
}

void Projectile::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   bool initial = false;

   if(stream->readFlag())
   {
      ((GameConnection *) connection)->readCompressedPoint(pos, stream);
      readCompressedVelocity(velocity, CompressedVelocityMax, stream);

      mType = stream->readEnum(TypeCount);

      if(stream->readFlag())
         mShooter = (Ship *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));
      pos += velocity * -0.020f;
      Rect newExtent(pos,pos);
      setExtent(newExtent);
      initial = true;
      SFXObject::play(SFXPhaser, pos, velocity);
   }
   bool preCollided = collided;
   collided = stream->readFlag();
   alive = stream->readFlag();

   if(!preCollided && collided)
      explode(NULL, pos);

   if(!collided && initial)
   {
      mCurrentMove.time = U32(connection->getOneWayTime());
     // idle(GameObject::ClientIdleMainRemote);
   }
}

void Projectile::handleCollision(GameObject *hitObject, Point collisionPoint)
{
   collided = true;

   if(!isGhost())
   {
      DamageInfo theInfo;
      theInfo.collisionPoint = collisionPoint;
      theInfo.damageAmount = gProjInfo[mType].damageAmount;
      theInfo.damageType = 0;
      theInfo.damagingObject = mShooter;
      theInfo.impulseVector = velocity;

      hitObject->damageObject(&theInfo);
   }

   mTimeRemaining = 0;
   explode(hitObject, collisionPoint);
}

void Projectile::idle(GameObject::IdleCallPath path)
{
   U32 deltaT = mCurrentMove.time;
   if(!collided && alive)
   {
      Point endPos = pos + velocity * deltaT * 0.001;
      static Vector<GameObject *> disableVector;

      Rect queryRect(pos, endPos);

      float collisionTime;
      disableVector.clear();

      if(mShooter.isValid() && mAliveTime < 100)
      {
         disableVector.push_back(mShooter);
         mShooter->disableCollision();
      }

      GameObject *hitObject;
      Point surfNormal;
      for(;;)
      {
         hitObject = findObjectLOS(MoveableType | BarrierType | EngineeredType | ForceFieldType, MoveObject::RenderState, pos, endPos, collisionTime, surfNormal);
         if(!hitObject || hitObject->collide(this))
            break;
         disableVector.push_back(hitObject);
         hitObject->disableCollision();
      }

      for(S32 i = 0; i < disableVector.size(); i++)
         disableVector[i]->enableCollision();

      if(hitObject)
      {
         if(mType == Bounce && hitObject->getObjectTypeMask() & BarrierType)
         {
            // test out reflection
            velocity -= surfNormal * surfNormal.dot(velocity) * 2;
            Point collisionPoint = pos + (endPos - pos) * collisionTime;
            pos = collisionPoint + surfNormal;
         }
         else
         {
            Point collisionPoint = pos + (endPos - pos) * collisionTime;
            handleCollision(hitObject, collisionPoint);
         }
      }
      else
         pos = endPos;

      Rect newExtent(pos,pos);
      setExtent(newExtent);
   }

   mAliveTime += deltaT;
   if(alive && path == GameObject::ServerIdleMainLoop)
   {
      if(mTimeRemaining <= deltaT)
      {
         deleteObject(500);
         mTimeRemaining = 0;
         alive = false;
         setMaskBits(ExplodedMask);
      }
      else
         mTimeRemaining -= deltaT;
   }
}

void Projectile::explode(GameObject *hitObject, Point thePos)
{
   // Do some particle spew...
   if(isGhost())
   {
      SparkManager::emitExplosion(thePos, 0.3, gProjInfo[mType].sparkColors, NumSparkColors);

      Ship *s = dynamic_cast<Ship*>(hitObject);
      if(s && s->isShieldActive())
         SFXObject::play(SFXBounceShield, thePos, velocity);
      else
         SFXObject::play(SFXPhaserImpact, thePos, velocity);
   }
}

void Projectile::render()
{
   if(collided || !alive)
      return;

   ProjectileInfo *pi = gProjInfo + mType;

   glColor3f(pi->projColors[0].r,pi->projColors[0].g,pi->projColors[0].b);
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);
   glScalef(pi->scaleFactor, pi->scaleFactor, 1);

   glPushMatrix();
   glRotatef((mAliveTime % 720) * 0.5, 0, 0, 1);

   glBegin(GL_LINE_LOOP);
   glVertex2f(-2, 2);
   glVertex2f(0, 6);
   glVertex2f(2, 2);
   glVertex2f(6, 0);
   glVertex2f(2, -2);
   glVertex2f(0, -6);
   glVertex2f(-2, -2);
   glVertex2f(-6, 0);
   glEnd();

   glPopMatrix();

   glRotatef(180 - (mAliveTime % 360), 0, 0, 1);
   glColor3f(pi->projColors[1].r,pi->projColors[1].g,pi->projColors[1].b);
   glBegin(GL_LINE_LOOP);
   glVertex2f(-2, 2);
   glVertex2f(0, 8);
   glVertex2f(2, 2);
   glVertex2f(8, 0);
   glVertex2f(2, -2);
   glVertex2f(0, -8);
   glVertex2f(-2, -2);
   glVertex2f(-8, 0);
   glEnd();

   glPopMatrix();
}

TNL_IMPLEMENT_NETOBJECT(GrenadeProjectile);

GrenadeProjectile::GrenadeProjectile(Point pos, Point vel, U32 liveTime, GameObject *shooter)
 : Item(pos, true, 7.f, 1.f)
{
   mObjectTypeMask = MoveableType;

   mNetFlags.set(Ghostable);

   mMoveState[0].pos = pos;
   mMoveState[0].vel = vel;
   setMaskBits(PositionMask);

   updateExtent();

   ttl = liveTime;
   exploded = false;
}

void GrenadeProjectile::idle(IdleCallPath path)
{
   Parent::idle(path);

   // Do some drag...
   mMoveState[0].vel -= mMoveState[0].vel * (F32(mCurrentMove.time) / 1000.f);

   if(!exploded)
   {
      if(getActualVel().len() < 4.0)
        explode(getActualPos());
   }

   if(isGhost()) return;

   // Update TTL
   U32 deltaT = mCurrentMove.time;
   if(path == GameObject::ClientIdleMainRemote)
      ttl += deltaT;
   else if(!exploded)
   {
      if(ttl <= deltaT)
        explode(getActualPos());
      else
         ttl -= deltaT;
   }

}


U32  GrenadeProjectile::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   U32 ret = Parent::packUpdate(connection, updateMask, stream);
   stream->writeFlag(exploded);
   return ret;
}

void GrenadeProjectile::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection, stream);

   if(stream->readFlag())
   {
      explode(getActualPos());
   }
}

void GrenadeProjectile::damageObject(DamageInfo *theInfo)
{
   // If we're being damaged by another grenade, explode...
   if(theInfo->damageType == 1)
   {
      explode(getActualPos());
      return;
   }

   // Bounce off of stuff.
   Point dv = theInfo->impulseVector - mMoveState[ActualState].vel;
   Point iv = mMoveState[ActualState].pos - theInfo->collisionPoint;
   iv.normalize();
   mMoveState[ActualState].vel += iv * dv.dot(iv) * 0.3;

   setMaskBits(PositionMask);
}

static Vector<GameObject *> fillVector;

void GrenadeProjectile::explode(Point pos)
{
   if(exploded) return;

   if(isGhost())
   {
      // Make us go boom!
      Color b(1,1,1);

      SparkManager::emitExplosion(getRenderPos(), 0.5, &b, 1);
   }

   disableCollision();

   if(!isGhost())
   {
      setMaskBits(PositionMask);
      deleteObject(100);

      DamageInfo info;
      info.collisionPoint = pos;
      info.damagingObject = this;
      info.damageAmount   = 0.5;
      info.damageType     = 1;

      radiusDamage(pos, 200.f, ShipType | EngineeredType, &info);
   }

   exploded = true;

}

void GrenadeProjectile::renderItem(Point pos)
{
   if(exploded)
      return;

   glColor3f(1,1,1);
   glBegin(GL_LINE_LOOP);
   for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.4)
      glVertex2f(pos.x + cos(theta) * 10.f, 
                 pos.y + sin(theta) * 10.f);
   glEnd();

   glColor3f(1,0,0);
   glBegin(GL_LINE_LOOP);
   for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.4)
      glVertex2f(pos.x + cos(theta) * 6.f, 
                 pos.y + sin(theta) * 6.f);
   glEnd();
}

};
