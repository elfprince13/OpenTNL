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

#ifndef _SFX_H_
#define _SFX_H_

#include "tnl.h"
#include "tnlNetBase.h"
#include "point.h"

using namespace TNL;

namespace Zap
{

enum SFXBuffers {
   SFXPhaser,
   SFXPhaserImpact,
   SFXShipExplode,
   SFXFlagCapture,
   SFXFlagDrop,
   SFXFlagReturn,
   SFXFlagSnatch,
   SFXTeleportIn,
   SFXTeleportOut,
   SFXBounceWall,
   SFXBounceObject,
   NumSFXBuffers
};

class SFXObject : public Object
{
   static Point mListenerPosition;
   static Point mListenerVelocity;
   static F32 mMaxDistance;

   U32 mSFXIndex;
   Point mPosition;
   Point mVelocity;
   bool mIsRelative;
   F32 mGain;
   bool mIsLooping;
   S32 mSourceIndex;
   F32 mPriority;
   void playOnSource();
   void updateGain();
   void updateMovementParams();
public:
   SFXObject(U32 sfxIndex, F32 gain = 1.0, bool looping = false); // constructor for a 2D sound
   SFXObject(U32 sfxIndex, Point position, Point velocity, F32 gain = 1.0, bool looping = false);
   ~SFXObject();

   void setGain(F32 gain);
   void play();
   void stop();
   void setLooping(bool looping);
   void setMovementParams(Point position, Point velocity);

   static void setMaxDistance(F32 maxDistance);
   static void init();
   static void shutdown();
   static void process();
   static void setListenerParams(Point position, Point velocity);
};

typedef RefPtr<SFXObject> SFXHandle;

extern void SFXInit();
extern void SFXShutdown();
extern void SFXProcess();
extern void SFXSetListenerParams(Point position, Point velocity);

};

#endif