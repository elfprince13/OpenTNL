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
#include "gameType.h"
#include "gameConnection.h"

#include <stdio.h>

namespace Zap
{

//------------------------------------------------------------------------
TNL_IMPLEMENT_NETOBJECT(Ship);

Ship::Ship(StringTableEntry playerName, Point p, F32 m) : MoveObject(p, CollisionRadius)
{
   mObjectTypeMask = ShipType | MoveableType | CommandMapVisType;

   mNetFlags.set(Ghostable);

   for(U32 i = 0; i < MoveStateCount; i++)
   {
      mMoveState[i].pos = p;
      mMoveState[i].angle = 0;
   }
   mHealth = 1.0;
   interpTime = 0;
   mass = m;
   hasExploded = false;
   timeUntilRemove = 0;
   updateExtent();
   lastFireTime = 0;

   mPlayerName = playerName;

   for(S32 i=0; i<TrailCount; i++)
      mTrail[i].reset();
}

void Ship::processArguments(S32 argc, const char **argv)
{
   if(argc != 5)
      return;

   mMoveState[0].pos.read(argv);

   for(U32 i = 1; i < MoveStateCount; i++)
   {
      mMoveState[i].pos = mMoveState[0].pos;
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
void Ship::processMove(Move *theMove, U32 stateIndex)
{
   U32 msTime = theMove->time;

   mMoveState[LastProcessState] = mMoveState[stateIndex];

   float time = theMove->time * 0.001;
   Point requestVel(theMove->right - theMove->left, theMove->down - theMove->up);
   requestVel *= MaxVelocity;
   F32 len = requestVel.len();

   if(len > MaxVelocity)
      requestVel *= MaxVelocity / len;

   Point velDelta = requestVel - mMoveState[stateIndex].vel;
   float accRequested = velDelta.len();

   float maxAccel = Acceleration * time;
   if(accRequested > maxAccel)
   {
      velDelta *= maxAccel / accRequested;
      mMoveState[stateIndex].vel += velDelta;
   }
   else
      mMoveState[stateIndex].vel = requestVel;

   mMoveState[stateIndex].angle = theMove->angle;
   move(time, stateIndex, false);
}

void Ship::processServerMove(Move *theMove)
{
   if(!hasExploded)
      processMove(theMove, ActualState);
   else
      return;

   mMoveState[RenderState] = mMoveState[ActualState];
   setMaskBits(PositionMask);
   if(!theMove->isEqualMove(&lastMove))
   {
      lastMove = *theMove;
      setMaskBits(MoveMask);
   }
   if(theMove->fire)
   {
      U32 currentTime = Platform::getRealMilliseconds();
      if(currentTime - lastFireTime > FireDelay)
      {
         lastFireTime = currentTime;
         Point dir(sin(mMoveState[ActualState].angle), cos(mMoveState[ActualState].angle) );
         Point projVel = dir * 600 + dir * mMoveState[ActualState].vel.dot(dir); //mMoveState[ActualState].vel + dir * 500;
         Projectile *proj = new Projectile(mMoveState[ActualState].pos + dir * (CollisionRadius-1), projVel, 500, this);
         proj->addToGame(getGame());
      }
   }
   updateExtent();
}

void Ship::damageObject(DamageInfo *theInfo)
{
   if(!getGame()->getGameType()->objectCanDamageObject(theInfo->damagingObject, this))
      return;

   mHealth -= theInfo->damageAmount;
   setMaskBits(HealthMask);
   if(mHealth <= 0)
   {
      mHealth = 0;
      kill(theInfo);
   }
   else if(mHealth > 1)
      mHealth = 1;
}

void Ship::processClientMove(Move *theMove, bool replay)
{
   if(!hasExploded)
      processMove(theMove, ActualState);
   else
      return;

   //mMoveState[RenderState] = mMoveState[ActualState];
   updateInterpolation(theMove->time);
   lastMove = *theMove;

   // Emit some particles
   emitMovementSparks();

   for(U32 i=0; i<TrailCount; i++)
      mTrail[i].tick(theMove->time);

   SFXObject::setListenerParams(mMoveState[RenderState].pos, mMoveState[RenderState].vel);
}

void Ship::processClient(U32 deltaT)
{
   lastMove.time = deltaT;
   processMove(&lastMove, ActualState);
   updateInterpolation(deltaT);
   // Emit some particles
   emitMovementSparks();

   for(U32 i=0; i<TrailCount; i++)
      mTrail[i].tick(deltaT);
}

void Ship::updateInterpolation(U32 deltaT)
{
   mMoveState[RenderState].angle = mMoveState[ActualState].angle;

   if(interpTime)
   {
      // first step is to constrain the render velocity to
      // the vector of difference between the current position and
      // the actual position.
      // we can also clamp to zero, the actual velocity, or the
      // render velocity, depending on which one is best.

      Point deltaP = mMoveState[ActualState].pos - mMoveState[RenderState].pos;
      F32 distance = deltaP.len();

      if(!distance)
         goto interpDone;

      deltaP.normalize();
      F32 vel = deltaP.dot(mMoveState[RenderState].vel);
      F32 avel = deltaP.dot(mMoveState[ActualState].vel);

      if(avel > vel)
         vel = avel;
      if(vel < 0)
         vel = 0;

      bool hit = true;
      float time = deltaT * 0.001;
      if(vel * time > distance)
         goto interpDone;

      float requestVel = distance / time;
      if(requestVel > InterpMaxVelocity)
      {
         hit = false;
         requestVel = InterpMaxVelocity;
      }
      F32 a = (requestVel - vel) / time;
      if(a > InterpAcceleration)
      {
         a = InterpAcceleration;
         hit = false;
      }

      if(hit)
         goto interpDone;

      vel += a * time;
      mMoveState[RenderState].vel = deltaP * vel;
      mMoveState[RenderState].pos += mMoveState[RenderState].vel * time;
   }
   else
   {
interpDone:
      interpTime = 0;
      mMoveState[RenderState] = mMoveState[ActualState];
   }
   updateExtent();
}

void Ship::processServer(U32 deltaT)
{
   lastMove.time = deltaT;
   if(isControlled())
      processMove(&lastMove, RenderState);
   else
   {
      processMove(&lastMove, ActualState);
      mMoveState[RenderState] = mMoveState[ActualState];
      setMaskBits(PositionMask);
   }
   updateExtent();
}

void Ship::writeControlState(BitStream *stream)
{
   stream->write(mMoveState[ActualState].pos.x);
   stream->write(mMoveState[ActualState].pos.y);
   stream->write(mMoveState[ActualState].angle);
   stream->write(mMoveState[ActualState].vel.x);
   stream->write(mMoveState[ActualState].vel.y);
   stream->write(mEnergy);
}

void Ship::readControlState(BitStream *stream)
{
   stream->read(&mMoveState[ActualState].pos.x);
   stream->read(&mMoveState[ActualState].pos.y);
   stream->read(&mMoveState[ActualState].angle);
   stream->read(&mMoveState[ActualState].vel.x);
   stream->read(&mMoveState[ActualState].vel.y);
   stream->read(&mEnergy);
   
   Point delta = mMoveState[ActualState].pos - mMoveState[RenderState].pos;
   if(delta.len() > MaxControlObjectInterpDistance)
   {
      for(S32 i=0; i<TrailCount; i++)
         mTrail[i].reset();

      interpTime = 0;
   }
   else
      interpTime = 1;
}

U32 Ship::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   if(stream->writeFlag(updateMask & InitialMask))
   {
      connection->packStringTableEntry(stream, mPlayerName);
      stream->write(mass);

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

   stream->writeFlag(hasExploded);

   bool shouldWritePosition = (updateMask & InitialMask) || 
      ((GameConnection *)connection)->controlObject != (GameObject *) this;

   if(!shouldWritePosition)
   {
      stream->writeFlag(false);
      stream->writeFlag(false);
   }
   else
   {
      if(stream->writeFlag(updateMask & PositionMask))
      {
         stream->write(mMoveState[RenderState].pos.x);
         stream->write(mMoveState[RenderState].pos.y);
         stream->write(mMoveState[RenderState].vel.x);
         stream->write(mMoveState[RenderState].vel.y);

         stream->writeFlag(updateMask & WarpPositionMask);
      }
      if(stream->writeFlag(updateMask & MoveMask))
      {
         lastMove.pack(stream, NULL);
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

   bool explode = stream->readFlag();
   if(stream->readFlag())
   {
      stream->read(&mMoveState[ActualState].pos.x);
      stream->read(&mMoveState[ActualState].pos.y);
      stream->read(&mMoveState[ActualState].vel.x);
      stream->read(&mMoveState[ActualState].vel.y);
      //posSegments.push_back(mMoveState[ActualState].pos);
      positionChanged = true;
      interpolate = !stream->readFlag();
   }
   if(stream->readFlag())
   {
      lastMove = Move();
      lastMove.unpack(stream);
   }
   mMoveState[ActualState].angle = lastMove.angle;

   if(positionChanged)
   {
      lastMove.time = connection->getOneWayTime();
      processMove(&lastMove, ActualState);

      if(interpolate)
      {
         interpTime = InterpMS;
         // if the actual velocity is in the direction of the actual position
         // then we'll set it into the render velocity
      }
      else
      {
         interpTime = 0;
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

void Ship::performInnerScopeQuery(GhostConnection *connection)
{
   Rect queryRect(mMoveState[ActualState].pos, mMoveState[ActualState].pos);
   queryRect.expand(Point(Game::PlayerHorizScopeDistance, Game::PlayerVertScopeDistance));


   fillVector.clear();
   findObjects(AllObjectTypes, fillVector, queryRect);

   for(S32 i = 0; i < fillVector.size(); i++)
      connection->objectInScope(fillVector[i]);
}

void Ship::performScopeQuery(GhostConnection *connection)
{
   GameConnection *g = dynamic_cast<GameConnection*>(connection);

   if(g && g->mInCommanderMap)
   {
      GameType *gt = gServerGame->getGameType();

      S32 teamId = gt->mClientList[gt->findClientIndexByConnection(g)].teamId;

      // Scope for all ships on our team.
      for(S32 i=0; i<gt->mClientList.size(); i++)
      {
         if(gt->mClientList[i].teamId == teamId)
         {
            Ship *co = dynamic_cast<Ship*>(gt->mClientList[i].clientConnection->getControlObject());
            if(co)
               co->performInnerScopeQuery(connection);
         }
      }
   }
   else
   {
      // Just scope for the current ship...
      performInnerScopeQuery(connection);
   }
}

F32 getAngleDiff(F32 a, F32 b)
{
   // Figure out the shortest path from a to b...
   // Restrict them to the range 0-360
   while(a<0)   a+=360;
   while(a>360) a-=360;

   while(b<0)   b+=360;
   while(b>360) b-=360;

   if(abs(b-a) > 180)
   {
      // Go the other way
      return  360-(b-a);
   }
   else
   {
      return b-a;
   }
}

void Ship::kill(DamageInfo *theInfo)
{
   if(isGhost())
      return;

   GameConnection *controllingClient = getControllingClient();
   getGame()->deleteObject(this, KillDeleteDelay);
   hasExploded = true;
   setMaskBits(ExplosionMask);
   disableCollision();

   if(controllingClient)
   {
      GameType *gt = getGame()->getGameType();
      if(gt)
         gt->controlObjectForClientKilled(controllingClient, this, theInfo->damagingObject);
   }
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
   // Do nothing if we're under 0.001 vel
   if(hasExploded || mMoveState[ActualState].vel.len() < 0.1)
      return;

   Point corners[3];
   Point shipDirs[3];

   corners[0].set(-20, -15);
   corners[1].set(  0,  25);
   corners[2].set( 20, -15);

   F32 th = mMoveState[RenderState].angle;

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
      mTrail[0].update(leftPt);
      mTrail[1].update(rightPt); 
      mLastTrailPoint[0] = leftId;
      mLastTrailPoint[1] = rightId;
   }
   else if(leftId == mLastTrailPoint[1] && rightId == mLastTrailPoint[0])
   {
      mTrail[1].update(leftPt);
      mTrail[0].update(rightPt);
      mLastTrailPoint[1] = leftId;
      mLastTrailPoint[0] = rightId;
   }
   else
   {
      mTrail[0].update(leftPt);
      mTrail[1].update(rightPt);
      mLastTrailPoint[0] = leftId;
      mLastTrailPoint[1] = rightId;
   }

}

void Ship::render()
{
   if(hasExploded)
      return;

   for(U32 i=0; i<TrailCount; i++)
      mTrail[i].render();

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

   // Render name...
   if(gClientGame->getConnectionToServer()->getControlObject() != this)
   {
      static char buff[255];
      sprintf(buff, "%s", mPlayerName.getString());

      // Make it a nice pastel
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1,1,1,0.5);
      //glColor3f(color.r*1.2,color.g*1.2,color.b*1.2);
      UserInterface::drawString( UserInterface::getStringWidth(14, buff) * -0.5, 30, 14, buff );
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
   }
   
   F32 alpha = 1.0;

//   if(hasExploded)
//   {
//      glEnable(GL_BLEND);
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//      alpha = timeUntilRemove / F32(ExplosionFadeTime);
//   }

   // draw thrusters
   Point velDir(lastMove.right - lastMove.left, lastMove.down - lastMove.up);
   F32 len = velDir.len();
   F32 thrusts[4];
   for(U32 i = 0; i < 4; i++)
      thrusts[i] = 0;

   if(len > 0)
   {
      if(len > 1)
         velDir *= 1 / len;

      Point shipDirs[4];
      shipDirs[0].set(sin(mMoveState[RenderState].angle), cos(mMoveState[RenderState].angle) );
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

   // first render the thrusters

   glRotatef(radiansToDegrees(mMoveState[RenderState].angle), 0, 0, -1.0);

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
   U32 health = 14 * mHealth;
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

   if(hasExploded)
   {
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
   }

   glPopMatrix();

   for(S32 i = 0; i < mMountedItems.size(); i++)
      if(mMountedItems[i].isValid())
         mMountedItems[i]->renderItem(mMoveState[RenderState].pos);

}

};