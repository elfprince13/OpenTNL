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

#include "engineeredObjects.h"
#include "ship.h"
#include "glutInclude.h"
#include "projectile.h"
#include "gameType.h"
namespace Zap
{

static Vector<GameObject *> fillVector;

void engClientCreateObject(GameConnection *connection, U32 object)
{
   Ship *ship = (Ship *) connection->getControlObject();
   if(!ship)
      return;

   if(!ship->carryingResource())
      return;

   Point startPoint = ship->getActualPos();
   Point endPoint = startPoint + ship->getAimVector() * Ship::MaxEngineerDistance;

   F32 collisionTime;
   Point collisionNormal;

   GameObject *hitObject = ship->findObjectLOS(BarrierType, 
      MoveObject::ActualState, startPoint, endPoint, collisionTime, collisionNormal);

   if(!hitObject)
      return;

   Point deployPosition = startPoint + (endPoint - startPoint) * collisionTime;

   // move the deploy point away from the wall by one unit...
   deployPosition += collisionNormal;

   EngineeredObject *deployedObject = NULL;
   switch(object)
   {
      case EngineeredTurret:
         deployedObject = new Turret(ship->getTeam(), deployPosition, collisionNormal);
         break;
      case EngineeredForceField:
         deployedObject = new ForceFieldProjector(ship->getTeam(), deployPosition, collisionNormal);
         break;
   }
   deployedObject->mOwner = ship;

   deployedObject->computeExtent();
   if(!deployedObject || !deployedObject->checkDeploymentPosition())
   {
      delete deployedObject;
      return;
   }
   deployedObject->addToGame(gServerGame);
   Item *theItem = ship->unmountResource();

   deployedObject->setResource(theItem);
}

EngineeredObject::EngineeredObject(S32 team, Point anchorPoint, Point anchorNormal)
{
   mHealth         = 1.f;
   mTeam           = team;
   mAnchorPoint    = anchorPoint;
   mAnchorNormal   = anchorNormal;
   mObjectTypeMask = EngineeredType | CommandMapVisType;
}

void EngineeredObject::setResource(Item *resource)
{
   TNLAssert(resource->isMounted() == false, "Doh!");
   mResource = resource;
   mResource->removeFromDatabase();
}

void EngineeredObject::damageObject(DamageInfo *di)
{
   mHealth -= di->damageAmount * .5f;

   if(mHealth > 0.f)
      return;

   mResource->addToDatabase();
   mResource->setActualPos(mAnchorPoint + mAnchorNormal * mResource->getRadius());

   getGame()->deleteObject(this, 0);
}

void EngineeredObject::computeExtent()
{
   Vector<Point> v;
   getCollisionPoly(v);
   Rect r(v[0], v[0]);
   for(S32 i = 1; i < v.size(); i++)
      r.unionPoint(v[i]);
   setExtent(r);
}

bool PolygonsIntersect(Vector<Point> &p1, Vector<Point> &p2)
{
   Point rp1 = p1[p1.size() - 1];
   for(S32 i = 0; i < p1.size(); i++)
   {
      Point rp2 = p1[i];

      Point cp1 = p2[p2.size() - 1];
      for(S32 j = 0; j < p2.size(); j++)
      {
         Point cp2 = p2[j];
         Point ce = cp2 - cp1;
         Point n(-ce.y, ce.x);

         F32 distToZero = n.dot(cp1);

         F32 d1 = n.dot(rp1);
         F32 d2 = n.dot(rp2);

         bool d1in = d1 >= distToZero;
         bool d2in = d2 >= distToZero;

         if(!d1in && !d2in) // both points are outside this edge of the poly, so...
            break;
         else if((d1in && !d2in) || (d2in && !d1in))
         {
            // find the clip intersection point:
            F32 t = (distToZero - d1) / (d2 - d1);
            Point clipPoint = rp1 + (rp2 - rp1) * t;

            if(d1in)
               rp2 = clipPoint;
            else
               rp1 = clipPoint;
         }
         else if(j == p2.size() - 1)
            return true;

         // if both are in, just go to the next edge.
         cp1 = cp2;
      }
      rp1 = rp2;
   }
   return false;
}

bool EngineeredObject::checkDeploymentPosition()
{
   Vector<GameObject *> go;
   Vector<Point> polyBounds;
   getCollisionPoly(polyBounds);

   gServerGame->getGridDatabase()->findObjects(BarrierType | EngineeredType, go, getExtent());
   for(S32 i = 0; i < go.size(); i++)
   {
      Vector<Point> compareBounds;
      go[i]->getCollisionPoly(compareBounds);
      if(PolygonsIntersect(polyBounds, compareBounds))
         return false;
   }
   return true;
}

U32 EngineeredObject::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   if(stream->writeFlag(updateMask & InitialMask))
   {
      stream->write(mTeam);
      stream->write(mAnchorPoint.x);
      stream->write(mAnchorPoint.y);
      stream->write(mAnchorNormal.x);
      stream->write(mAnchorNormal.y);
   }
   return 0;
}

void EngineeredObject::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   if(stream->readFlag())
   {
      stream->read(&mTeam);
      stream->read(&mAnchorPoint.x);
      stream->read(&mAnchorPoint.y);
      stream->read(&mAnchorNormal.x);
      stream->read(&mAnchorNormal.y);
      computeExtent();
   }
}

TNL_IMPLEMENT_NETOBJECT(ForceFieldProjector);

ForceFieldProjector::~ForceFieldProjector()
{
   if(mField.isValid())
      getGame()->deleteObject(mField, 0);
}

bool ForceFieldProjector::getCollisionPoly(Vector<Point> &polyPoints)
{
   Point cross(mAnchorNormal.y, -mAnchorNormal.x);
   polyPoints.push_back(mAnchorPoint + cross * 30);
   polyPoints.push_back(mAnchorPoint + cross * 20 + mAnchorNormal * 15);
   polyPoints.push_back(mAnchorPoint - cross * 20 + mAnchorNormal * 15);
   polyPoints.push_back(mAnchorPoint - cross * 30);
   return true;
}

void ForceFieldProjector::render()
{
   Vector<Point> p;
   getCollisionPoly(p);
   glColor3f(1,1,1);
   glBegin(GL_LINE_LOOP);
   for(S32 i = 0; i < p.size(); i++)
      glVertex2f(p[i].x, p[i].y);
   glEnd();
}

void ForceFieldProjector::idle(IdleCallPath path)
{
   if(path == ServerIdleMainLoop)
   {
      mFieldDown.update(mCurrentMove.time);

      if(mFieldDown.getCurrent() == 0)
      {
         // Choose a target...
         Point pos = mAnchorPoint;

         Rect queryRect(mAnchorPoint, mAnchorPoint + mAnchorNormal * 800.f * mLength);
         queryRect.expand(Point(20, 20));

         fillVector.clear();
         findObjects(ShipType, fillVector, queryRect);

         bool foundFriendly = false;

         for(S32 i=0; i<fillVector.size(); i++)
         {
            Ship *potential = (Ship*)fillVector[i];

            // Is it dead or cloaked?
            if(potential->isCloakActive() || potential->hasExploded)
               continue;

            // Is it on our team?
            if(potential->getTeam() != mTeam)
               continue;

            // Is it near our field?


            foundFriendly = true;
         }

         if(foundFriendly)
         {
            // Destroy the barrier...
            if(mField.isValid())
            {
               getGame()->deleteObject(mField, 15);
            }
         }
         else
         {
            if(mField.isNull())
            {
               Point n;
               if(!findObjectLOS(BarrierType, 0, mAnchorPoint + mAnchorNormal * 0.1, mAnchorPoint + mAnchorNormal * 800.f, mLength, n))
                  mLength = 1.f;

               mField = new Barrier(mAnchorPoint + mAnchorNormal * 20.f, mAnchorPoint + mAnchorNormal * 800.f * mLength);
               mField->addToGame(getGame());
            }
         }

         mFieldDown.reset(FieldDownTime);
      }
   }
}

TNL_IMPLEMENT_NETOBJECT(Turret);

bool Turret::getCollisionPoly(Vector<Point> &polyPoints)
{
   Point cross(mAnchorNormal.y, -mAnchorNormal.x);
   polyPoints.push_back(mAnchorPoint + cross * 30);
   polyPoints.push_back(mAnchorPoint + cross * 20 + mAnchorNormal * 15);
   polyPoints.push_back(mAnchorPoint - cross * 20 + mAnchorNormal * 15);
   polyPoints.push_back(mAnchorPoint - cross * 30);
   return true;
}

void Turret::render()
{
   Vector<Point> p;
   getCollisionPoly(p);
   
   Color teamColor;

   if(gClientGame->getGameType())
      teamColor = gClientGame->getGameType()->mTeams[mTeam].color;
   else
      teamColor = Color(1,0,1);

   glColor3f(teamColor.r, teamColor.g, teamColor.b);
   glBegin(GL_LINE_LOOP);
   for(S32 i = 0; i < p.size(); i++)
      glVertex2f(p[i].x, p[i].y);
   glEnd();
}

void Turret::idle(IdleCallPath path)
{
   if(path == ServerIdleMainLoop)
   {
      mFireTimer.update(mCurrentMove.time);

      if(mFireTimer.getCurrent() == 0)
      {
         // Choose a target...
         Point pos = mAnchorPoint;

         Rect queryRect(pos, pos);
         queryRect.expand(Point(800, 800));

         fillVector.clear();
         findObjects(ShipType, fillVector, queryRect);

         Ship * bestTarget = NULL;
         F32 bestRange = 10000.f;
         Point bestDelta;

         Point delta;

         F32 timeScale = F32(mCurrentMove.time) / 1000.f;

         for(S32 i=0; i<fillVector.size(); i++)
         {
            Ship *potential = (Ship*)fillVector[i];

            // Is it dead or cloaked?
            if(potential->isCloakActive() || potential->hasExploded)
               continue;

            // Is it on our team?
            if(potential->getTeam() == mTeam)
               continue;

            // Calculate where we have to shoot to hit this...
            const F32 projVel = 600.f;
            F32 shipVel = potential->getActualVel().len() * 0.8;

            // If we can't hit 'em (or the math will break, move on), pretend we're trying
            if(shipVel > projVel)
               shipVel = projVel;

            Point distVec  = potential->getActualPos() - pos;
            F32 travelTime = distVec.len() / sqrt( projVel * projVel - shipVel*shipVel );
            Point leadPos  = potential->getActualPos() + potential->getActualVel() * travelTime;

            // Calculate distance
            delta = (leadPos - pos);

            // Check that we're facing it...
            if(delta.dot(mAnchorNormal) <= 0.f)
               continue;

            // See if we can see it...
            F32 t;
            Point n;
            if(findObjectLOS(BarrierType, 0, mAnchorPoint, potential->getActualPos(), t, n))
               continue;

            // See if we're gonna clobber our own stuff...
            disableCollision();
            Point delta2 = delta;
            delta2.normalize(600.f * 1.f);
            EngineeredObject *g = dynamic_cast<EngineeredObject*>(findObjectLOS(EngineeredType, 0, mAnchorPoint, mAnchorPoint + delta2, t, n));
            enableCollision();

            if(g && g->getTeam() == mTeam)
               continue;

            F32 dist = delta.len();

            if(dist < bestRange)
            {
               bestDelta  = delta;
               bestRange  = dist;
               bestTarget = potential;
            }
         }

         if(bestTarget)
         {
            bestDelta.normalize();
            Projectile *proj = new Projectile(pos + bestDelta * 30.f, bestDelta * 600.f, 500, mOwner);
            proj->addToGame(gServerGame);
         }

         mFireTimer.reset(90);
      }
   }

}

};