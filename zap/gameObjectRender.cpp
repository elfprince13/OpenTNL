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

#include "gameObjectRender.h"
#include "glutInclude.h"
#include "tnlRandom.h"

namespace Zap
{

void glVertex(Point p)   
{
   glVertex2f(p.x, p.y);
}


void renderShip(Color c, F32 alpha, F32 thrusts[], F32 health, F32 radius, bool cloakActive, bool shieldActive)
{
   if(alpha != 1.0)
      glEnable(GL_BLEND);
   
   if(cloakActive)
   {
      glColor4f(0,0,0, 1 - alpha);
      glBegin(GL_POLYGON);
      glVertex2f(-20, -15);
      glVertex2f(0, 25);
      glVertex2f(20, -15);
      glEnd();
   }

   // first render the thrusters

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
   
   glColor4f(c.r,c.g,c.b, alpha);
   glBegin(GL_LINE_LOOP);
   glVertex2f(-12, -13);
   glVertex2f(0, 22);
   glVertex2f(12, -13);
   glEnd();

   U32 lineCount = U32(14 * health);
   glBegin(GL_LINES);
   for(U32 i = 0; i < lineCount; i++)
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

   // Render shield if appropriate
   if(shieldActive)
   {
      F32 shieldRadius = radius + 3;

      glColor4f(1,1,0, alpha);
      glBegin(GL_LINE_LOOP);
      for(F32 theta = 0; theta <= 2 * 3.1415; theta += 0.3)
         glVertex2f(cos(theta) * shieldRadius, sin(theta) * shieldRadius);
      
      glEnd();
   }


   if(alpha != 1.0)
      glDisable(GL_BLEND);
}

void renderTeleporter(U32 time, F32 radiusFraction, F32 radius, F32 alpha)
{
   enum {
      NumColors = 6,
      NumParticles = 250,
   };

   static bool trackerInit = false;

   struct Tracker
   {
      F32 thetaI;
      F32 thetaP;
      F32 dI;
      F32 dP;
      U32 ci;
   };
   static Tracker particles[NumParticles];

   static float colors[NumColors][3] = {
      { 0, 1, 0 },
      { 0, 0.5, 1 },
      { 0, 0, 1 },
      { 0, 1, 1 },
      { 0, 1, 0.5 },
      { 0, 0, 1 },
   };
   if(!trackerInit)
   {
      trackerInit = true;
      for(S32 i = 0; i < NumParticles; i++)
      {
         Tracker &t = particles[i];
         t.thetaI = Random::readF() * Float2Pi;
         t.thetaP = Random::readF() * 2 + 0.5;
         t.dP = Random::readF() * 10 + 5;
         t.dI = Random::readF() * t.dP;
         t.ci = Random::readI(0, NumColors - 1);
      }
   }

   glBegin(GL_LINES);
   F32 arcLen = 0.3 + (1 - radiusFraction) * 0.8;
   F32 white = (1 - radiusFraction) * alpha;
   for(S32 i = 0; i < NumParticles; i++)
   {
      Tracker &t = particles[i];
      //glColor3f(t.c.r, t.c.g, t.c.b);
      glColor3f(colors[t.ci][0] * alpha * radiusFraction + white, 
                colors[t.ci][1] * alpha * radiusFraction + white,
                colors[t.ci][2] * alpha * radiusFraction + white);
      F32 theta = fmod(float( t.thetaI + time * 0.001 * t.thetaP), (float) Float2Pi);
      F32 d = fmod(float(t.dI + time * 0.001), (float) t.dP);
      Point p(cos(theta), sin(theta));
      Point n(-p.y, p.x);

      p *= radiusFraction * radius * (t.dP - d) / t.dP;
      glVertex2f(p.x, p.y);

      glColor3f(0,0,0);
      theta -= arcLen;
      p.set(cos(theta), sin(theta));
      p *= radiusFraction * radius * (t.dP - d) / t.dP;
      glVertex2f(p.x, p.y);

      //glVertex2f(p.x - n.x * 5, p.y - n.y * 5);
   }
   glEnd();
}

void renderTurret(Color c, Point anchor, Point normal, bool enabled, F32 health, F32 barrelAngle, F32 aimOffset)
{
   glColor3f(c.r, c.g, c.b);

   Point cross(normal.y, -normal.x);
   Point aimCenter = anchor + normal * aimOffset;

   glBegin(GL_LINE_STRIP);

   for(S32 x = -10; x <= 10; x++)
   {
      F32 theta = x * FloatHalfPi * 0.1;
      Point pos = normal * cos(theta) + cross * sin(theta);
      glVertex(aimCenter + pos * 15);
   }
   glEnd();

   glLineWidth(3);
   glBegin(GL_LINES);
   Point aimDelta(cos(barrelAngle), sin(barrelAngle));
   glVertex(aimCenter + aimDelta * 15);
   glVertex(aimCenter + aimDelta * 30);
   glEnd();
   glLineWidth(1);

   if(enabled)
      glColor3f(1,1,1);
   else
      glColor3f(0.6, 0.6, 0.6);
   glBegin(GL_LINE_LOOP);
   glVertex(anchor + cross * 18);
   glVertex(anchor + cross * 18 + normal * aimOffset);
   glVertex(anchor - cross * 18 + normal * aimOffset);
   glVertex(anchor - cross * 18);
   glEnd();

   glColor3f(c.r, c.g, c.b);
   S32 lineHeight = U32(28 * health);
   glBegin(GL_LINES);
   for(S32 i = 0; i < lineHeight; i += 2)
   {
      Point lsegStart = anchor - cross * (14 - i) + normal * 5;
      Point lsegEnd = lsegStart + normal * (aimOffset - 10);
      glVertex(lsegStart);
      glVertex(lsegEnd);
   }
   Point lsegStart = anchor - cross * 14 + normal * 3;
   Point lsegEnd = anchor + cross * 14 + normal * 3;
   Point n = normal * (aimOffset - 6);
   glVertex(lsegStart);
   glVertex(lsegEnd);
   glVertex(lsegStart + n);
   glVertex(lsegEnd + n);
   glEnd();
}


};