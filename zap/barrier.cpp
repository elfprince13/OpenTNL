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

#include "barrier.h"
#include "glutInclude.h"
#include "gameLoader.h"
#include <math.h>

using namespace TNL;

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(Barrier);

U32 Barrier::mBarrierChangeIndex = 1;

class BarrierMaker : public GameObject
{
public:
   void processArguments(S32 argc, const char **argv);
   TNL_DECLARE_CLASS(BarrierMaker);
};

TNL_IMPLEMENT_CLASS(BarrierMaker);

void BarrierMaker::processArguments(S32 argc, const char **argv)
{
   Point lastPoint;

   Vector<Point> vec;
   bool loop;

   for(S32 i = 1; i < argc; i += 2)
   {
      float x = (float) atof(argv[i-1]) * getGame()->getGridSize();
      float y = (float) atof(argv[i]) * getGame()->getGridSize();
      vec.push_back(Point(x,y));
   }
   if(vec.size() < 1)
      return;

   loop = vec[0] == vec[vec.size() - 1];

   Vector<Point> edgeVector;
   for(S32 i = 0; i < vec.size() - 1; i++)
   {
      Point e = vec[i+1] - vec[i];
      e.normalize();
      edgeVector.push_back(e);
   }

   Point lastEdge = edgeVector[edgeVector.size() - 1];
   Vector<F32> extend;
   for(S32 i = 0; i < edgeVector.size(); i++)
   {
      Point curEdge = edgeVector[i];
      double cosTheta = curEdge.dot(lastEdge);
      
      if(cosTheta >= 0)
      {
         F32 extendAmt = Barrier::BarrierWidth * 0.5 * tan( acos(cosTheta) / 2 );
         extend.push_back(extendAmt);
      }
      else
         extend.push_back(0);
      lastEdge = curEdge;
   }
   extend.push_back(extend[0]);

   for(S32 i = 0; i < edgeVector.size(); i++)
   {
      F32 extendBack = extend[i];
      F32 extendForward = extend[i+1];
      if(i == 0 && !loop)
         extendBack = 0;
      if(i == edgeVector.size() - 1 && !loop)
         extendForward = 0;

      Point start = vec[i] - edgeVector[i] * extendBack;
      Point end = vec[i+1] + edgeVector[i] * extendForward;
      Barrier *b = new Barrier(start, end);
      b->addToGame(getGame());
   }
}

Barrier::Barrier(Point st, Point e)
{
   mObjectTypeMask = BarrierType;
   mNetFlags.set(Ghostable);
   start = st;
   end = e;
   Rect r(start, end);
   r.expand(Point(BarrierWidth, BarrierWidth));
   setExtent(r);
   mLastBarrierChangeIndex = 0;
}

U32 Barrier::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   stream->write(start.x);
   stream->write(start.y);
   stream->write(end.x);
   stream->write(end.y);
   return 0;
}

void Barrier::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   stream->read(&start.x);
   stream->read(&start.y);
   stream->read(&end.x);
   stream->read(&end.y);

   Rect r(start, end);
   r.expand(Point(BarrierWidth, BarrierWidth));
   setExtent(r);
   mBarrierChangeIndex++;
}

bool Barrier::getCollisionPoly(Vector<Point> &polyPoints)
{
   Point vec = end - start;
   Point crossVec(vec.y, -vec.x);
   crossVec.normalize(BarrierWidth * 0.5);
   
   polyPoints.push_back(Point(start.x + crossVec.x, start.y + crossVec.y));
   polyPoints.push_back(Point(end.x + crossVec.x, end.y + crossVec.y));
   polyPoints.push_back(Point(end.x - crossVec.x, end.y - crossVec.y));
   polyPoints.push_back(Point(start.x - crossVec.x, start.y - crossVec.y));
   return true;
}

void Barrier::clipRenderLinesToPoly(Vector<Point> &polyPoints)
{
   Vector<Point> clippedSegments;

   for(S32 i = 0; i < mRenderLineSegments.size(); i+= 2)
   {
      Point rp1 = mRenderLineSegments[i];
      Point rp2 = mRenderLineSegments[i + 1];
      
      Point cp1 = polyPoints[polyPoints.size() - 1];
      for(S32 j = 0; j < polyPoints.size(); j++)
      {
         Point cp2 = polyPoints[j];
         Point ce = cp2 - cp1;
         Point n(-ce.y, ce.x);

         n.normalize();
         F32 distToZero = n.dot(cp1);

         F32 d1 = n.dot(rp1);
         F32 d2 = n.dot(rp2);

         bool d1in = d1 > distToZero;
         bool d2in = d2 > distToZero;

         if(!d1in && !d2in) // both points are outside this edge of the poly, so...
         {
            // add them to the render poly
            clippedSegments.push_back(rp1);
            clippedSegments.push_back(rp2);
            break;
         }
         else if((d1in && !d2in) || (d2in && !d1in))
         {
            // find the clip intersection point:
            F32 t = (distToZero - d1) / (d2 - d1);
            Point clipPoint = rp1 + (rp2 - rp1) * t;

            if(d1in)
            {
               clippedSegments.push_back(clipPoint);
               clippedSegments.push_back(rp2);
               rp2 = clipPoint;
            }
            else
            {
               clippedSegments.push_back(rp1);
               clippedSegments.push_back(clipPoint);
               rp1 = clipPoint;
            }
         }

         // if both are in, just go to the next edge.

         cp1 = cp2;
      }
   }
   mRenderLineSegments = clippedSegments;
}

void Barrier::render()
{
   if(mLastBarrierChangeIndex != mBarrierChangeIndex)
   {
      mLastBarrierChangeIndex = mBarrierChangeIndex;
      mRenderLineSegments.clear();

      Vector<Point> colPoly;
      getCollisionPoly(colPoly);
      S32 last = colPoly.size() - 1;
      for(S32 i = 0; i < colPoly.size(); i++)
      {
         mRenderLineSegments.push_back(colPoly[last]);
         mRenderLineSegments.push_back(colPoly[i]);
         last = i;
      }
      static Vector<GameObject *> fillObjects;
      fillObjects.clear();

      Rect bounds(start, end);
      bounds.expand(Point(BarrierWidth, BarrierWidth));
      findObjects(BarrierType, fillObjects, bounds);

      for(S32 i = 0; i < fillObjects.size(); i++)
      {
         colPoly.clear();
         if(fillObjects[i] != this && fillObjects[i]->getCollisionPoly(colPoly))
            clipRenderLinesToPoly(colPoly);
      }
   }
   glColor3f(0,0,1);
   glBegin(GL_LINES);
   for(S32 i = 0; i < mRenderLineSegments.size(); i++)
      glVertex2f(mRenderLineSegments[i].x, mRenderLineSegments[i].y);
   glEnd();
}

};