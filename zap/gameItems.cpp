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

public:
   RepairItem(Point p = Point()) : PickupItem(p, 20)
   {
      mNetFlags.set(Ghostable);
   }

   bool pickup(Ship *theShip)
   {
      if(theShip->getHealth() >= 1)
         return false;

      DamageInfo di;
      di.damageAmount = -0.5f;
      di.damageType = 0;
      di.damagingObject = this;

      theShip->damageObject(&di);
      return true;
   }

   void onClientPickup()
   {
      SFXObject::play(SFXShipHeal, getRenderPos(), getRenderVel());
   }

   U32 getRepopDelay()
   {
      return 20000;
   }

   void renderItem(Point pos)
   {
      if(!isVisible())
         return;

      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0);

      glColor3f(1,1,1);
      glBegin(GL_LINE_LOOP);
      glVertex2f(-18, -18);
      glVertex2f(18, -18);
      glVertex2f(18, 18);
      glVertex2f(-18, 18);
      glEnd();

      glColor3f(1,0,0);
      glBegin(GL_LINE_LOOP);

      float crossWidth = 4;
      float crossLen = 14;

      glVertex2f(crossWidth, crossWidth);
      glVertex2f(crossLen, crossWidth);
      glVertex2f(crossLen, -crossWidth);
      glVertex2f(crossWidth, -crossWidth);
      glVertex2f(crossWidth, -crossLen);
      glVertex2f(-crossWidth, -crossLen);
      glVertex2f(-crossWidth, -crossWidth);
      glVertex2f(-crossLen, -crossWidth);
      glVertex2f(-crossLen, crossWidth);
      glVertex2f(-crossWidth, crossWidth);
      glVertex2f(-crossWidth, crossLen);
      glVertex2f(crossWidth, crossLen);
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