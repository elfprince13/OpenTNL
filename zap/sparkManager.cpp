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

#include "sparkManager.h"
#include "glutInclude.h"
using namespace TNL;

namespace Zap
{

namespace SparkManager
{

struct Spark
{
   Point pos;
   Color color;
   F32 alpha;
   F32 ttl;
   Point vel;
};

enum {
   MaxSparks = 4096,
};

U32 firstFreeIndex = 0;
Spark gSparks[MaxSparks];

void emitSpark(Point pos, Point vel, Color color, F32 ttl)
{
   if(firstFreeIndex >= MaxSparks)
      return;
   Spark *s = gSparks + firstFreeIndex;
   firstFreeIndex++;

   s->pos = pos;
   s->vel = vel;
   s->color = color;
   
   if(!ttl)
      s->ttl = 15 * Random::readF() * Random::readF();
   else
      s->ttl = ttl;
}


void tick( F32 dT )
{
   for(U32 i = 0; i < firstFreeIndex; )
   {
      Spark *theSpark = gSparks + i;
      if(theSpark->ttl <= dT)
      {
         firstFreeIndex--;
         *theSpark = gSparks[firstFreeIndex];
      }
      else
      {
         theSpark->ttl -= dT;
         theSpark->pos += theSpark->vel * dT;
         if(theSpark->ttl > 2)
            theSpark->alpha = 1;
         else
            theSpark->alpha = theSpark->ttl * 0.5;
         i++;
      }
   }
}

void SparkManager::render()
{
   glPointSize( 2.0f );
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   glEnableClientState(GL_COLOR_ARRAY);
   glEnableClientState(GL_VERTEX_ARRAY);

   glVertexPointer(2, GL_FLOAT, sizeof(Spark), &gSparks[0].pos);
   glColorPointer(4, GL_FLOAT , sizeof(Spark), &gSparks[0].color);
   
   glDrawArrays(GL_POINTS, 0, firstFreeIndex);

   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);
}

void emitExplosion(Point pos, F32 size)
{
   for(U32 i = 0; i < (500.0 * size); i++)
   {

      F32 th = Random::readF() * 2 * 3.14;
      F32 f = (Random::readF() * 2 - 1) * 400 * size;
      F32 green = Random::readF();

      emitSpark(pos, Point(cos(th)*f, sin(th)*f), Color(1, green, 0), Random::readF()*size + 2*size);
   }
}

void emitExplosion(Point pos, F32 size, Color *colorArray, U32 numColors)
{
   for(U32 i = 0; i < (500.0 * size); i++)
   {

      F32 th = Random::readF() * 2 * 3.14;
      F32 f = (Random::readF() * 2 - 1) * 400 * size;
      F32 green = Random::readF();
      U32 colorIndex = Random::readI() % numColors;

      emitSpark(pos, Point(cos(th)*f, sin(th)*f), colorArray[colorIndex], Random::readF()*size + 2*size);
   }
}

void emitBurst(Point pos, Point scale, Color color1, Color color2)
{
   F32 size = 1;

   for(U32 i = 0; i < (250.0 * size); i++)
   {

      F32 th = Random::readF() * 2 * 3.14;
      F32 f = (Random::readF() * 0.1 + 0.9) * 200 * size;
      F32 t = Random::readF();

      Color r;

      r.interp(t, color1, color2);

      emitSpark(
         pos + Point(cos(th)*scale.x, sin(th)*scale.y),
         Point(cos(th)*scale.x*f, sin(th)*scale.y*f),
         r,
         Random::readF() * scale.len() * 3 + scale.len()
      );
   }
}

};

};