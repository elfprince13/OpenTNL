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

#include "item.h"
#include "ship.h"

#include "glutInclude.h"

namespace Zap
{

Item::Item(Point p, bool collideable, float radius, float mass) : MoveObject(p, radius, mass)
{
   mIsMounted = false;
   mIsCollideable = collideable;
   mInterpTime = 0;
   mObjectTypeMask = MoveableType | ItemType;
}

void Item::processArguments(S32 argc, const char **argv)
{
   if(argc < 2)
      return;
   Point pos;
   pos.read(argv);
   pos *= getGame()->getGridSize();
   for(U32 i = 0; i < MoveStateCount; i++)
      mMoveState[i].pos = pos;
}

void Item::render()
{
   // if the item is mounted, renderItem will be called from the
   // ship it is mounted to
   if(mIsMounted)
      return;

   renderItem(mMoveState[RenderState].pos);
}

void Item::mountToShip(Ship *theShip)
{
   dismount();
   mMount = theShip;
   if(theShip)
      theShip->mMountedItems.push_back(this);

   mIsMounted = true;
   setMaskBits(MountMask);
}

void Item::dismount()
{
   if(mMount.isValid())
   {
      for(S32 i = 0; i < mMount->mMountedItems.size(); i++)
      {
         if(mMount->mMountedItems[i].getPointer() == this)
         {
            mMount->mMountedItems.erase(i);
            break;
         }
      }
   }
   mMount = NULL;
   mIsMounted = false;
   setMaskBits(MountMask);
}

void Item::processServer(U32 deltaT)
{
   if(mIsMounted)
   {
      if(mMount.isNull() || mMount->hasExploded)
         dismount();
      else
      {
         mMoveState[RenderState].pos = mMount->getRenderPos();
         mMoveState[ActualState].pos = mMount->getActualPos();

         updateExtent();
      }
   }
   else
   {
      float time = deltaT * 0.001f;
      move(time, ActualState, false);
      mMoveState[RenderState] = mMoveState[ActualState];
      setMaskBits(PositionMask);
      updateExtent();
   }
}

void Item::processClient(U32 deltaT)
{
   if(mIsMounted)
   {
      if(mMount.isValid())
      {
         mMoveState[RenderState].pos = mMount->getRenderPos();
         mMoveState[ActualState].pos = mMount->getActualPos();

         updateExtent();
      }
   }
   else
   {
      U32 timeUsed = deltaT;
      if(mInterpTime)
      {
         if(mInterpTime < timeUsed)
         {
            timeUsed -= mInterpTime;
            mInterpTime = 0;
            mMoveState[RenderState] = mMoveState[ActualState];
         }
         else
         {
            Point totalDelta = mMoveState[ActualState].pos -
                              mMoveState[RenderState].pos;

            mMoveState[RenderState].pos +=
                  totalDelta * (timeUsed / F32(mInterpTime));

            mInterpTime -= timeUsed;
            timeUsed = 0;
         }
      }
      if(timeUsed)
      {
         move(timeUsed * 0.001f, ActualState, false);
         mMoveState[RenderState] = mMoveState[ActualState];
      }
      updateExtent();
   }
}

U32 Item::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   U32 retMask = 0;
   if(stream->writeFlag(updateMask & InitialMask))
   {
   }
   if(stream->writeFlag(updateMask & PositionMask))
   {
      stream->write(mMoveState[RenderState].pos.x);
      stream->write(mMoveState[RenderState].pos.y);
      stream->write(mMoveState[RenderState].angle);
      stream->write(mMoveState[RenderState].vel.x);
      stream->write(mMoveState[RenderState].vel.y);
      stream->writeFlag(updateMask & WarpPositionMask);
   }
   if(stream->writeFlag(MountMask) && stream->writeFlag(mIsMounted))
   {
      S32 index = connection->getGhostIndex(mMount);
      if(stream->writeFlag(index != -1))
         stream->writeInt(index, GhostConnection::GhostIdBitSize);
      else
         retMask = MountMask;
   }
   return MountMask;
}

#define InterpMS 50
void Item::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   bool interpolate = false;
   bool positionChanged = false;

   if(stream->readFlag())
   {
   }
   if(stream->readFlag())
   {
      stream->read(&mMoveState[ActualState].pos.x);
      stream->read(&mMoveState[ActualState].pos.y);
      stream->read(&mMoveState[ActualState].angle);
      stream->read(&mMoveState[ActualState].vel.x);
      stream->read(&mMoveState[ActualState].vel.y);
      //posSegments.push_back(mMoveState[ActualState].pos);
      positionChanged = true;
      interpolate = !stream->readFlag();
   }
   if(stream->readFlag())
   {
      bool shouldMount = stream->readFlag();
      if(shouldMount)
      {
         Ship *theShip = NULL;
         if(stream->readFlag())
            theShip = (Ship *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));
         mountToShip(theShip);
      }
      else
         dismount();
   }
   
   if(positionChanged)
   {
      if(interpolate)
      {
         mInterpTime = InterpMS;
         move((mInterpTime + connection->getOneWayTime()) * 0.001f, ActualState, false);
      }
      else
      {
         mInterpTime = 0;
         mMoveState[RenderState] = mMoveState[ActualState];
      }
      //processMove(&lastMove, mMoveState[ActualState]);
   }
}

bool Item::collide(GameObject *otherObject)
{
   return mIsCollideable && !mIsMounted;
}

TNL_IMPLEMENT_NETOBJECT(TestItem);

TestItem::TestItem() : Item(Point(0,0), true, 60, 4)
{
   mNetFlags.set(Ghostable);
}

void TestItem::renderItem(Point pos)
{
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);

   glColor3f(1, 1, 0);
   glBegin(GL_LINE_LOOP);

   glVertex2f(-60, 0);
   glVertex2f(-40, 40);
   glVertex2f(0, 60);
   glVertex2f(40, 40);
   glVertex2f(60, 0);
   glVertex2f(40, -40);
   glVertex2f(0, -60);
   glVertex2f(-40, -40);

   glEnd();
   glPopMatrix();
}

void TestItem::damageObject(DamageInfo *theInfo)
{
   // compute impulse direction
   Point dv = theInfo->impulseVector - mMoveState[ActualState].vel;
   Point iv = mMoveState[ActualState].pos - theInfo->collisionPoint;
   iv.normalize();
   mMoveState[ActualState].vel += iv * dv.dot(iv) * 0.3;
}

};