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

#include "teleporter.h"
#include "glutInclude.h"

using namespace TNL;
#include "ship.h"
#include "sparkManager.h"
#include "gameLoader.h"
#include "sfx.h"

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(Teleporter);

Teleporter::Teleporter(Point start, Point end)
{
   mNetFlags.set(Ghostable);
   pos = start;
   dest = end;
   timeout = 0;
   spinFactor = 0.f;

   Rect r(pos, pos);
   r.expand(Point(TeleporterRadius, TeleporterRadius));
   setExtent(r);
   mObjectTypeMask |= CommandMapVisType;
}

void Teleporter::onAddedToGame(Game *theGame)
{
   if(!isGhost())
      setScopeAlways();
}

void Teleporter::processArguments(S32 argc, const char **argv)
{
   if(argc != 4)
      return;

   pos.read(argv);
   pos *= getGame()->getGridSize();

   dest.read(argv + 2);
   dest *= getGame()->getGridSize();

   Rect r(pos, pos);
   r.expand(Point(TeleporterRadius, TeleporterRadius));
   setExtent(r);
}

U32 Teleporter::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   bool isInitial = (updateMask & BIT(3));

   if(stream->writeFlag(updateMask & InitMask))
   {
      stream->write(pos.x);
      stream->write(pos.y);
      stream->write(dest.x);
      stream->write(dest.y);
   }

   stream->writeFlag((updateMask & TeleportMask) && !isInitial);

   return 0;
}

void Teleporter::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   if(stream->readFlag())
   {
      stream->read(&pos.x);
      stream->read(&pos.y);
      stream->read(&dest.x);
      stream->read(&dest.y);

      Rect r(pos, pos);
      r.expand(Point(TeleporterRadius, TeleporterRadius));
      setExtent(r);
   }
   if(stream->readFlag() && isGhost())
   {
      for(F32 th = 0; th < 360.0f; th+= Random::readF() * 3.f)
      {
         Point off(cos(th), sin(th));

         SparkManager::emitSpark(pos,               off * Random::readF() *  50.f, Color(0,1,0), 5);
         SparkManager::emitSpark(dest + off * 100,  off * Random::readF() * -50.f, Color(0,1,0), 5);

      }
      SFXObject::play(SFXTeleportIn, dest, Point());
      SFXObject::play(SFXTeleportOut, pos, Point());
      timeout = TeleporterDelay;
   }
}

static Vector<GameObject *> fillVector2;

void Teleporter::idle(GameObject::IdleCallPath path)
{
   U32 deltaT = mCurrentMove.time;
   // Deal with our timeout...
   if(timeout > deltaT)
   { 
      timeout -= deltaT;
      return;
   }
   else
      timeout = 0;

   // Update our spin...
   spinFactor += 10 * F32(deltaT)/1000.f;
   while(spinFactor > 720.f)
      spinFactor -= 720.f;

   if(path != GameObject::ServerIdleMainLoop)
      return;

   // Check for players within range
   // if so, blast them to dest
   Rect queryRect(pos, pos);
   queryRect.expand(Point(TeleporterRadius, TeleporterRadius));

   fillVector2.clear();
   findObjects(ShipType, fillVector2, queryRect);

   // First see if we're triggered...
   bool isTriggered = false;

   for(S32 i=0; i<fillVector2.size(); i++)
   {
      Ship *s = (Ship*)fillVector2[i];
      if((pos - s->getActualPos()).len() < TeleporterTriggerRadius)
      {
         isTriggered = true;
         setMaskBits(TeleportMask);
         timeout = TeleporterDelay;
      }
   }
   
   if(!isTriggered)
      return;

   for(S32 i=0; i<fillVector2.size(); i++)
   {
      Ship *s = (Ship*)fillVector2[i];
      if((pos - s->getActualPos()).len() < TeleporterRadius)
      {
         Point center = s->getActualPos() - pos + dest;
         F32 r = TNL::Random::readF();
         F32 theta = TNL::Random::readF() * 2 * 3.145926535;
         Point offset(cos(theta), sin(theta));
         // square root will give a uniform distribution, but
         // do a cube root in order to push the offset towards
         // the extents of the disk a bit more
         // (http://mathworld.wolfram.com/DiskPointPicking.html)
         offset *= pow(r, 1.0f / 3) * TeleporterRadius;
         s->setActualPos(center + offset);
      }
   }
}

inline Point polarToRect(Point p)
{
   F32 &r  = p.x;
   F32 &th = p.y;

   return Point(
      cos(th) * r,
      sin(th) * r
      );

}

void Teleporter::render()
{
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);
   glRotatef(spinFactor, 0, 0, 1);

   glColor3f(0.5, 1, 0.5);

   F32 r = F32(TeleporterDelay - timeout) / F32(TeleporterDelay);

   // Draw spirals (line segs) with three points
   //    - point 1 a little ways out from center
   //    - point 2 at trigger radius, rotated from 1
   //    - point 3 at teleport radius, rotated from 2

   // We'll do 7 arms...
   for(U32 i=0; i<7; i++)
   {
      const F32 step = (2.f*3.14f/7.f);
      const F32 th = F32(i) * step;
      
      Point in2 ( 5*r,                        th );
      Point mid2( TeleporterTriggerRadius*r,  th + 0.8 * step);
      Point out2( TeleporterRadius*r,         th + step);

      Point in  = polarToRect(in2);
      Point mid = polarToRect(mid2);
      Point out = polarToRect(out2);

      glBegin(GL_LINE_STRIP);
         glColor3f(0, 0, 1);
         glVertex2f( in.x, in.y );

         glColor3f(0, 1, 1);
         glVertex2f( mid.x, mid.y );

         glColor3f(0.9, 0.9, 0.9);
         glVertex2f( out.x, out.y );
      glEnd();
   }

   glVertex2f(-r,  r);
   glVertex2f(-r, -r);
   glVertex2f( r, -r);
   glVertex2f( r,  r);

   glPopMatrix();
}

};
