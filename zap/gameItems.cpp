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

class RepairItem : public PickupItem
{
private:
   typedef PickupItem Parent;
   F32 spin;

public:
   RepairItem(Point p = Point()) : PickupItem(p, 20)
   {
      spin=0.f;
      mNetFlags.set(Ghostable);
   }

   bool pickup(Ship *theShip)
   {
      if(theShip->getHealth() >= 1)
         return false;

      DamageInfo di;
      di.damageAmount = -0.21;
      di.damageType = 0;
      di.damagingObject = this;

      theShip->damageObject(&di);
      SFXObject::play(SFXShipHeal, getRenderPos(), getRenderVel());
      return true;
   }

   U32 getRepopDelay()
   {
      return 20000;
   }

   static void emitVerts(F32 x, F32 y)
   {
      static int verts[6][2] = {
         { 1, 0 },
         { 2, 1 },
         { 3, 1 },
         { 4, 2 },
         { 4, 3 },
         { 3, 2 } };
      glBegin(GL_LINE_STRIP);
      for(S32 i = 0; i < 6; i++)
         glVertex2f(x * verts[i][0], y * verts[i][1]);
      for(S32 i = 5; i >= 0; i--)
         glVertex2f(x * verts[i][1], y * verts[i][0]);
      glEnd();
   }

   void processClient(U32 deltaT)
   {
      Parent::processClient(deltaT);
      spin += 50.f * (F32)deltaT / 1000.f;
   }

   void renderItem(Point pos)
   {

      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0);
      glRotatef(spin, 0, 0, 1.f);

      glColor3f(1, 1, 0);
      
      emitVerts(-5, 5);
      emitVerts(5, 5);
      emitVerts(5, -5);
      emitVerts(-5, -5);
      glBegin(GL_LINES);
      glVertex2f(-5, 0);
      glVertex2f(0, -5);
      glVertex2f(5, 0);
      glVertex2f(0, 5);
      glEnd();

      glPopMatrix();
   }

   TNL_DECLARE_CLASS(RepairItem);
};

TNL_IMPLEMENT_NETOBJECT(RepairItem);

class TestItem : public Item
{
   U32 teamIndex;
public:
   TestItem() : Item(Point(0,0), true, 60, 4)
   {
      mNetFlags.set(Ghostable);
   }

   void renderItem(Point pos)
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

   void damageObject(DamageInfo *theInfo)
   {
      // compute impulse direction
      Point dv = theInfo->impulseVector - mMoveState[ActualState].vel;
      Point iv = mMoveState[ActualState].pos - theInfo->collisionPoint;
      iv.normalize();
      mMoveState[ActualState].vel += iv * dv.dot(iv) * 0.3;
   }

   TNL_DECLARE_CLASS(TestItem);
};

TNL_IMPLEMENT_NETOBJECT(TestItem);


};