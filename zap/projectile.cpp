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

TNL_IMPLEMENT_NETOBJECT(Projectile);

Projectile::Projectile(Point p, Point v, U32 t, Ship *shooter)
{
   mNetFlags.set(Ghostable);
   pos = p;
   velocity = v;
   liveTime = t;
   collided = false;
   alive = true;
   mShooter = shooter;
}

U32 Projectile::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   ((GameConnection *) connection)->writeCompressedPoint(pos, stream);
   writeCompressedVelocity(velocity, CompressedVelocityMax, stream);

   S32 index = -1;
   if(mShooter.isValid())
      index = connection->getGhostIndex(mShooter);
   if(stream->writeFlag(index != -1))
      stream->writeInt(index, GhostConnection::GhostIdBitSize);
   stream->writeFlag(collided);

   return 0;
}

void Projectile::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   ((GameConnection *) connection)->readCompressedPoint(pos, stream);
   readCompressedVelocity(velocity, CompressedVelocityMax, stream);

   if(stream->readFlag())
      mShooter = (Ship *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));

   collided = stream->readFlag();
   if(collided)
      explode(NULL, pos);
   else
      pos += velocity * -0.020f;

   Rect newExtent(pos,pos);
   setExtent(newExtent);
   mCurrentMove.time = U32(connection->getOneWayTime());
   idle(GameObject::ClientIdleMainRemote);

   SFXObject::play(0, pos, velocity);
}

enum {
   NumSparkColors = 4,
};

Color SparkColors[NumSparkColors] = {
Color(1, 0, 1),
Color(1, 1, 1),
Color(0, 0, 1),
Color(1, 0, 0),
};

void Projectile::handleCollision(GameObject *hitObject, Point collisionPoint)
{
   collided = true;

   if(!isGhost())
   {
      DamageInfo theInfo;
      theInfo.collisionPoint = collisionPoint;
      theInfo.damageAmount = 0.21;
      theInfo.damageType = 0;
      theInfo.damagingObject = mShooter;
      theInfo.impulseVector = velocity;

      hitObject->damageObject(&theInfo);
   }

   liveTime = 0;
   explode(hitObject, collisionPoint);
}

void Projectile::explode(GameObject *hitObject, Point thePos)
{
   // Do some particle spew...
   if(isGhost())
   {
      SparkManager::emitExplosion(thePos, 0.3, SparkColors, NumSparkColors);

      Ship *s = dynamic_cast<Ship*>(hitObject);
      if(s && s->isShieldActive())
         SFXObject::play(SFXBounceShield, thePos, velocity);
      else
         SFXObject::play(SFXPhaserImpact, thePos, velocity);
   }
}

void Projectile::idle(GameObject::IdleCallPath path)
{
   U32 deltaT = mCurrentMove.time;
   if(!collided)
   {
      Point endPos = pos + velocity * deltaT * 0.001;
      static Vector<GameObject *> disableVector;

      Rect queryRect(pos, endPos);

      float collisionTime;
      disableVector.clear();

      if(mShooter.isValid())
      {
         disableVector.push_back(mShooter);
         mShooter->disableCollision();
      }

      GameObject *hitObject;
      Point surfNormal;
      for(;;)
      {
         hitObject = findObjectLOS(MoveableType | BarrierType | EngineeredType, MoveObject::RenderState, pos, endPos, collisionTime, surfNormal);
         if(!hitObject || hitObject->collide(this))
            break;
         disableVector.push_back(hitObject);
         hitObject->disableCollision();
      }

      for(S32 i = 0; i < disableVector.size(); i++)
         disableVector[i]->enableCollision();

      if(hitObject)
      {
         //if(hitObject->getObjectTypeMask() & BarrierType)
         //{
         //   // test out reflection
         //   velocity -= surfNormal * surfNormal.dot(velocity) * 2;
        // }
         //else
         //{
            Point collisionPoint = pos + (endPos - pos) * collisionTime;
            handleCollision(hitObject, collisionPoint);
         //}
      }
      else
         pos = endPos;

      Rect newExtent(pos,pos);
      setExtent(newExtent);
   }

   if(path == GameObject::ClientIdleMainRemote)
      liveTime += deltaT;
   else if(alive)
   {
      if(liveTime <= deltaT)
      {
         getGame()->deleteObject(this, 500);
         liveTime = 0;
         alive = false;
      }
      else
         liveTime -= deltaT;
   }
}

void Projectile::render()
{
   if(collided)
      return;

   glColor3f(1,0,0.5);
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);

   glPushMatrix();
   glRotatef((liveTime % 720) * 0.5, 0, 0, 1);

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

   glRotatef(180 - (liveTime % 360), 0, 0, 1);
   glColor3f(0.5,0,1);
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

GrenadeProjectile::GrenadeProjectile(Point pos, Point vel, U32 liveTime, Ship *shooter)
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
      explode(getActualPos());

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

      SparkManager::emitExplosion(getRenderPos(), 1.0, &b, 1);
   }

   disableCollision();

   if(!isGhost())
   {
      setMaskBits(PositionMask);
      getGame()->deleteObject(this, 100);
   }

   exploded = true;

   DamageInfo info;
   info.collisionPoint = pos;
   info.damagingObject = this;
   info.damageAmount   = 0.5;
   info.damageType     = 1;

   // Check for players within range
   // if so, blast them to death
   Rect queryRect(pos, pos);
   queryRect.expand(Point(150, 150));

   fillVector.clear();
   findObjects(0xFFFFFFFF, fillVector, queryRect);

   for(S32 i=0; i<fillVector.size(); i++)
   {
      // figure the impulse
      info.impulseVector  = fillVector[i]->getActualPos() - pos;
      info.impulseVector.normalize();

      info.collisionPoint  = fillVector[i]->getActualPos();
      info.collisionPoint -= info.impulseVector;

      info.impulseVector  *= 2000.f;

      fillVector[i]->damageObject(&info);
   }      
}

void GrenadeProjectile::renderItem(Point pos)
{
   if(exploded)
      return;

/*   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0); */

   glColor3f(1,1,1);
   glBegin(GL_LINE_LOOP);
   for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.3)
      glVertex2f(pos.x + cos(theta) * 10.f, 
                 pos.y + sin(theta) * 10.f);
   glEnd();

   glColor3f(1,0,0);
   glBegin(GL_LINE_LOOP);
   for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.3)
      glVertex2f(pos.x + cos(theta) * 6.f, 
                 pos.y + sin(theta) * 6.f);
   glEnd();

//   glPopMatrix();
}

};
