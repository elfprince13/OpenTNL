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

#include "sfx.h"
#include "tnl.h"
#include "tnlLog.h"

#if defined (TNL_OS_WIN32) || defined (TNL_OS_LINUX) || defined (TNL_OS_MAC_OSX)

#include "alInclude.h"

using namespace TNL;

namespace Zap
{

static SFXProfile gSFXProfiles[] = {
 {  "phaser.wav", false, 0.45f, false, 150, 600 },
 {  "phaser_impact.wav", false, 0.7f, false, 150, 600 },
 {  "ship_explode.wav", false, 1.0, false, 300, 1000 },
 {  "flag_capture.wav", true, 0.45f, false, 0, 0 },
 {  "flag_drop.wav", true, 0.45f, false, 0, 0 },
 {  "flag_return.wav", true, 0.45f, false, 0, 0 },
 {  "flag_snatch.wav", true, 0.45f, false, 0, 0 }, 
 {  "teleport_in.wav", false, 1.0, false, 200, 500 },
 {  "teleport_out.wav", false, 1.0, false, 200, 500 },
 {  "bounce_wall.wav", false, 0.7f, false, 150, 600 },
 {  "bounce_obj.wav", false, 0.7f, false, 150, 600 },
 {  NULL, false, 0, false, 0, 0 },
};

static ALCdevice *gDevice = NULL;
static ALCcontext *gContext = NULL;
static bool gSFXValid = false;

enum {
   NumSources = 16,
};

static ALuint gSources[NumSources];
static bool gSourceActive[NumSources];
Point SFXObject::mListenerPosition;
Point SFXObject::mListenerVelocity;
F32 SFXObject::mMaxDistance = 500;

static ALuint gBuffers[NumSFXBuffers];
static Vector<SFXHandle> gPlayList;

SFXObject::SFXObject(U32 profileIndex, F32 gain, Point position, Point velocity)
{
   mSFXIndex = profileIndex;
   mProfile = gSFXProfiles + profileIndex;
   mGain = gain;
   mPosition = position;
   mVelocity = velocity;
   mSourceIndex = -1;
   mPriority = 0;
}

RefPtr<SFXObject> SFXObject::play(U32 profileIndex, F32 gain)
{
   RefPtr<SFXObject> ret = new SFXObject(profileIndex, gain, Point(), Point());
   ret->play();
   return ret;
}

RefPtr<SFXObject> SFXObject::play(U32 profileIndex, Point position, Point velocity, F32 gain)
{
   RefPtr<SFXObject> ret = new SFXObject(profileIndex, gain, position, velocity);
   ret->play();
   return ret;
}

SFXObject::~SFXObject()
{

}

void SFXObject::updateGain()
{
   ALuint source = gSources[mSourceIndex];
   F32 gain = mGain;

   if(!mProfile->isRelative)
   {
      F32 distance = (mListenerPosition - mPosition).len();
      if(distance > mProfile->fullGainDistance)
      {
         if(distance < mProfile->zeroGainDistance)
            gain *= (mProfile->fullGainDistance - distance) / 
                    (mProfile->zeroGainDistance - mProfile->fullGainDistance);
         else
            gain = 0.0f;
      }
      else
         gain = 1.0f;
   }
   else
      gain = 1.0f;

   alSourcef(source, AL_GAIN, gain * mProfile->gainScale);
}

void SFXObject::updateMovementParams()
{
   ALuint source = gSources[mSourceIndex];
   if(mProfile->isRelative)
   {
      alSourcei(source, AL_SOURCE_RELATIVE, true);
      alSource3f(source, AL_POSITION, 0, 0, 0);
      //alSource3f(source, AL_VELOCITY, 0, 0, 0);
   }
   else
   {
      alSourcei(source, AL_SOURCE_RELATIVE, false);
      alSource3f(source, AL_POSITION, mPosition.x, mPosition.y, 0);
      //alSource3f(source, AL_VELOCITY, mVelocity.x, mVelocity.y, 0);
   }
}

void SFXObject::playOnSource()
{
   ALuint source = gSources[mSourceIndex];
   alSourceStop(source);
   alSourcei(source, AL_BUFFER, gBuffers[mSFXIndex]);
   alSourcei(source, AL_LOOPING, mProfile->isLooping);
   alSourcef(source, AL_REFERENCE_DISTANCE,9000);
   alSourcef(source, AL_ROLLOFF_FACTOR,1);
   alSourcef(source, AL_MAX_DISTANCE, 10000);
   updateMovementParams();

   updateGain();
   alSourcePlay(source);
}

void SFXObject::setGain(F32 gain)
{
   mGain = gain;
   if(mSourceIndex != -1)
      updateGain();
}

void SFXObject::setMovementParams(Point position, Point velocity)
{
   mPosition = position;
   mVelocity = velocity;
   if(mSourceIndex != -1)
      updateMovementParams();
}

void SFXObject::play()
{
   if(mSourceIndex != -1)
      return;
   else
   {
      // see if it's on the play list:
      S32 i;
      for(i = 0; i < gPlayList.size(); i++)
         if(this == gPlayList[i].getPointer())
            return;
      gPlayList.push_back(this);
   }
}

void SFXObject::stop()
{
   // remove from the play list, if this sound is playing:
   if(mSourceIndex != -1)
   {
      alSourceStop(gSources[mSourceIndex]);
      mSourceIndex = -1;
   }
   for(S32 i = 0; i < gPlayList.size(); i++)
   {
      if(gPlayList[i].getPointer() == this)
      {
         gPlayList.erase(i);
         return;
      }
   }
}

void SFXObject::init()
{
   ALint error;
   gDevice = alcOpenDevice((ALubyte *) "DirectSound3D");
   if(!gDevice)
   {
      logprintf("Failed to intitialize OpenAL.");
      return;
   }

   static int contextData[][2] =
   {
      {ALC_FREQUENCY, 11025},
      {0,0} // Indicate end of list...
   };

   gContext = alcCreateContext(gDevice, (ALCint*)contextData);
   alcMakeContextCurrent(gContext);

   error = alGetError();

   alGenBuffers(NumSFXBuffers, gBuffers);
   error = alGetError();

   alDistanceModel(AL_NONE);
   error = alGetError();

   // load up all the sound buffers
   //if(error != AL_NO_ERROR)
   //   return;

   alGenSources(NumSources, gSources);

   for(U32 i = 0; i < NumSFXBuffers; i++)
   {
      if(!gSFXProfiles[i].fileName)
         break;

      ALsizei size,freq;
      ALenum   format;
      ALvoid   *data;
      ALboolean loop;

#ifdef TNL_OS_MAC_OSX
      alutLoadWAVFile((ALbyte *) gSFXProfiles[i].fileName, &format, &data, &size, &freq);
#else
      alutLoadWAVFile((ALbyte *) gSFXProfiles[i].fileName, &format, &data, &size, &freq, &loop);
#endif
      if(alGetError() != AL_NO_ERROR)
         return;
      alBufferData(gBuffers[i], format, data, size, freq);
      alutUnloadWAV(format, data, size, freq);
      if(alGetError() != AL_NO_ERROR)
         return;
   }
   gSFXValid = true;
}

void SFXObject::process()
{
   if(!gSFXValid)
      return;

   // ok, so we have a list of currently "playing" sounds, which is
   // unbounded in length, but only the top NumSources actually have sources
   // associtated with them.  Sounds are prioritized on a 0-1 scale
   // based on type and distance.
   // Each time through the main loop, newly played sounds are placed
   // on the process list.  When SFXProcess is called, any finished sounds
   // are retired from the list, and then it prioritizes and sorts all
   // the remaining sounds.  For any sounds from 0 to NumSources that don't
   // have a current source, they grab one of the sources not used by the other
   // top sounds.  At this point, any sound that is not looping, and is
   // not in the active top list is retired.

   // ok, look through all the currently playing sources and see which
   // ones need to be retired:

   for(S32 i = 0; i < NumSources; i++)
   {
      ALint state;
      alGetSourcei(gSources[i], AL_SOURCE_STATE, &state);
      gSourceActive[i] = state != AL_STOPPED && state != AL_INITIAL;
   }
   for(S32 i = 0; i < gPlayList.size(); )
   {
      SFXHandle &s = gPlayList[i];
      if(s->mSourceIndex != -1 && !gSourceActive[s->mSourceIndex])
      {
         // this sound was playing; now it is stopped,
         // so remove it from the list.
         s->mSourceIndex = -1;
         gPlayList.erase_fast(i);
      }
      else
      {
         // compute a priority for this sound.
         if(!s->mProfile->isRelative)
            s->mPriority = (500 - (s->mPosition - mListenerPosition).len()) / 500.0f;
         else
            s->mPriority = 1.0;
         i++;
      }
   }
   // now, bubble sort all the sounds up the list:
   // we choose bubble sort, because the list should
   // have a lot of frame-to-frame coherency, making the
   // sort most often O(n)
   for(S32 i = 1; i < gPlayList.size(); i++)
   {
      F32 priority = gPlayList[i]->mPriority;
      for(S32 j = i - 1; j >= 0; j--)
      {
         if(priority > gPlayList[j]->mPriority)
         {
            SFXHandle temp = gPlayList[j];
            gPlayList[j] = gPlayList[j+1];
            gPlayList[j+1] = temp;
         }
      }
   }
   // last, release any sources and get rid of non-looping sounds
   // outside our max sound limit
   for(S32 i = NumSources; i < gPlayList.size(); )
   {
      SFXHandle &s = gPlayList[i];
      if(s->mSourceIndex != -1)
      {
         gSourceActive[s->mSourceIndex] = false;
         s->mSourceIndex = -1;
      }
      if(!s->mProfile->isLooping)
         gPlayList.erase_fast(i);
      else
         i++;
   }
   // now assign sources to all our sounds that need them:
   S32 firstFree = 0;
   S32 max = NumSources;
   if(max > gPlayList.size())
      max = gPlayList.size();

   for(S32 i = 0; i < max; i++)
   {
      SFXHandle &s = gPlayList[i];
      if(s->mSourceIndex == -1)
      {
         while(gSourceActive[firstFree])
            firstFree++;
         s->mSourceIndex = firstFree;
         gSourceActive[firstFree] = true;
         s->playOnSource();
      }
      else
      {
         // for other sources, just attenuate the gain.
         s->updateGain();
      }
   }
}

void SFXObject::setListenerParams(Point pos, Point velocity)
{
   if(!gSFXValid)
      return;

   mListenerPosition = pos;
   mListenerVelocity = velocity;
   alListener3f(AL_POSITION, pos.x, pos.y, -mMaxDistance/2);
}

void SFXObject::shutdown()
{
   if(!gSFXValid)
      return;

   alDeleteBuffers(NumSFXBuffers, gBuffers);
   alcMakeContextCurrent(NULL);
   alcDestroyContext(gContext);
   alcCloseDevice(gDevice);
}

};

#else

using namespace TNL;

namespace Zap
{

Point SFXObject::mListenerPosition;
Point SFXObject::mListenerVelocity;
F32 SFXObject::mMaxDistance = 500;

SFXObject::SFXObject(U32 sfxIndex, F32 gain, bool looping)
{
}

SFXObject::SFXObject(U32 sfxIndex, Point position, Point velocity, F32 gain, bool looping)
{
}

SFXObject::~SFXObject()
{

}

void SFXObject::updateGain()
{
}

void SFXObject::updateMovementParams()
{
}

void SFXObject::playOnSource()
{
}

void SFXObject::setGain(F32 gain)
{
}

void SFXObject::setLooping(bool looping)
{
}

void SFXObject::setMovementParams(Point position, Point velocity)
{
}

void SFXObject::play()
{
}

void SFXObject::stop()
{
}

void SFXObject::init()
{
   logprintf("No OpenAL support on this platform.");
}

void SFXObject::process()
{
 }

void SFXObject::setListenerParams(Point pos, Point velocity)
{
}

void SFXObject::shutdown()
{
};

};

#endif
