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

#ifdef TNL_OS_WIN32

#include <dsound.h>
#include <stdio.h>

namespace Zap
{

static LPDIRECTSOUNDCAPTURE8 capture = NULL;
static LPDIRECTSOUNDCAPTUREBUFFER captureBuffer = NULL;
static bool recording = false;

enum {
   BufferBytes = 16000,
};

static U32 lastReadOffset = 0;

void SFXObject::setCaptureGain(S32 amt)
{
   mCaptureGain = amt;
}

bool SFXObject::startRecording()
{
   if(recording)
      return true;

   if(!capture)
   {
      DirectSoundCaptureCreate8(NULL, &capture, NULL);   
      if(!capture)
         return false;
   }

   if(!captureBuffer)
   {
      HRESULT hr;
      DSCBUFFERDESC dscbd;

      // wFormatTag, nChannels, nSamplesPerSec, mAvgBytesPerSec,
      // nBlockAlign, wBitsPerSample, cbSize
      WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 16000, 2, 16, 0};
       
      dscbd.dwSize = sizeof(DSCBUFFERDESC);
      dscbd.dwFlags = 0;
      dscbd.dwBufferBytes = BufferBytes;
      dscbd.dwReserved = 0;
      dscbd.lpwfxFormat = &wfx;
      dscbd.dwFXCount = 0;
      dscbd.lpDSCFXDesc = NULL;
       
      if (FAILED(hr = capture->CreateCaptureBuffer(&dscbd, &captureBuffer, NULL)))
         return false;
   }
   recording = true;
   lastReadOffset = 0;
   captureBuffer->Start(DSCBSTART_LOOPING);
   return true;
}

void SFXObject::captureSamples(ByteBufferPtr buffer)
{
   if(!recording)
   {
      return;
   }
   else
   {
      DWORD capturePosition;
      DWORD readPosition;

      captureBuffer->GetCurrentPosition(&capturePosition, &readPosition);
      S32 byteCount = readPosition - lastReadOffset;
      if(byteCount < 0)
         byteCount += BufferBytes;

      void *buf1;
      void *buf2;
      DWORD count1;
      DWORD count2;

      printf("Capturing samples... %d ... %d\n", lastReadOffset, readPosition);

      if(!byteCount)
         return;

      captureBuffer->Lock(lastReadOffset, byteCount, &buf1, &count1, &buf2, &count2, 0);

      U32 sizeAdd = count1 + count2;
      U32 start = buffer->getBufferSize();

      buffer->resize(start + sizeAdd);

      memcpy(buffer->getBuffer() + start, buf1, count1);
      if(count2)
         memcpy(buffer->getBuffer() + start + count1, buf2, count2);

      captureBuffer->Unlock(buf1, count1, buf2, count2);

      // Endeavour futilely to do a gain pass
      S16 *data = (S16*)buffer->getBuffer();
      for(U32 i=start/2; i<(start+sizeAdd)/2; i++)
         data[i] <<= mCaptureGain;
      
      lastReadOffset += count1 + count2;
      lastReadOffset %= BufferBytes;
   }
}

void SFXObject::stopRecording()
{
   if(recording)
   {
      recording = false;
      captureBuffer->Stop();
      if(captureBuffer)
         captureBuffer->Release();
      captureBuffer = NULL;
   }
}

};

#else

namespace Zap
{

bool SFXObject::startRecording()
{
   return false;
}

void SFXObject::captureSamples(ByteBufferPtr buffer)
{
}

void SFXObject::stopRecording()
{
}

};

#endif

#if defined (TNL_OS_WIN32) || defined (TNL_OS_LINUX) || defined (TNL_OS_MAC_OSX)

#include "alInclude.h"

using namespace TNL;

namespace Zap
{

static SFXProfile gSFXProfiles[] = {
 {  "phaser.wav",          true,  1.0f,  false, 0,   0 }, 
 {  "phaser.wav",          false, 0.45f, false, 150, 600 },
 {  "phaser_impact.wav",   false, 0.7f,  false, 150, 600 },
 {  "ship_explode.wav",    false, 1.0,   false, 300, 1000 },
 {  "ship_heal.wav",       false, 1.0,   false, 300, 1000 },
 {  "ship_turbo.wav",      false, 1.0,   true,  150, 500 },
 {  "flag_capture.wav",    true,  0.45f, false, 0,   0 },
 {  "flag_drop.wav",       true,  0.45f, false, 0,   0 },
 {  "flag_return.wav",     true,  0.45f, false, 0,   0 },
 {  "flag_snatch.wav",     true,  0.45f, false, 0,   0 }, 
 {  "teleport_in.wav",     false, 1.0,   false, 200, 500 },
 {  "teleport_out.wav",    false, 1.0,   false, 200, 500 },
 {  "bounce_wall.wav",     false, 0.7f,  false, 150, 600 },
 {  "bounce_obj.wav",      false, 0.7f,  false, 150, 600 },
 {  "bounce_shield.wav",   false, 0.7f,  false, 150, 600 },
 {  "boop.wav",            true,  0.4f,  false, 150, 600 },
 {  "comm_up.wav",         true,  0.4f,  false, 150, 600 },
 {  "comm_down.wav",       true,  0.4f,  false, 150, 600 },
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
static bool gQueuedBuffers[NumSources] = { false, };
Point SFXObject::mListenerPosition;
Point SFXObject::mListenerVelocity;
F32 SFXObject::mMaxDistance = 500;
S32 SFXObject::mCaptureGain = 0;

static ALuint gBuffers[NumSFXBuffers];
static Vector<ALuint> gVoiceFreeBuffers;

static Vector<SFXHandle> gPlayList;

SFXObject::SFXObject(U32 profileIndex, ByteBufferPtr ib, F32 gain, Point position, Point velocity)
{
   mSFXIndex = profileIndex;
   mProfile = gSFXProfiles + profileIndex;
   mGain = gain;
   mPosition = position;
   mVelocity = velocity;
   mSourceIndex = -1;
   mPriority = 0;
   mInitialBuffer = ib;
}

RefPtr<SFXObject> SFXObject::play(U32 profileIndex, F32 gain)
{
   RefPtr<SFXObject> ret = new SFXObject(profileIndex, NULL, gain, Point(), Point());
   ret->play();
   return ret;
}

RefPtr<SFXObject> SFXObject::play(U32 profileIndex, Point position, Point velocity, F32 gain)
{
   RefPtr<SFXObject> ret = new SFXObject(profileIndex, NULL, gain, position, velocity);
   ret->play();
   return ret;
}

RefPtr<SFXObject> SFXObject::playRecordedBuffer(ByteBufferPtr p, F32 gain)
{
   RefPtr<SFXObject> ret = new SFXObject(0, p, gain, Point(), Point());
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


static void unqueueBuffers(S32 sourceIndex)
{
   // free up any played buffers from this source.
   if(sourceIndex != -1)
   {
      ALint processed;
      alGetSourcei(gSources[sourceIndex], AL_BUFFERS_PROCESSED, &processed);
      while(processed)
      {
         ALuint buffer;
         alSourceUnqueueBuffers(gSources[sourceIndex], 1, &buffer);
         logprintf("unqueued buffer %d\n", buffer);
         processed--;
         
         // ok, this is a lame solution - but the way OpenAL should work is...
         // you should only be able to unqueue buffers that you queued - duh!
         // otherwise it's a bitch to manage sources that can either be streamed
         // or already loaded.
         U32 i;
         for(i = 0 ; i < NumSFXBuffers; i++)
            if(buffer == gBuffers[i])
               break;
         if(i == NumSFXBuffers)
            gVoiceFreeBuffers.push_back(buffer);
      }
   }
}

void SFXObject::queueBuffer(ByteBufferPtr p)
{
   mInitialBuffer = p;
   if(mSourceIndex != -1)
   {
      ALuint source = gSources[mSourceIndex];
      ALuint buffer = gVoiceFreeBuffers.first();
      gVoiceFreeBuffers.pop_front();

      S16 max = 0;
      S16 *ptr = (S16 *) p->getBuffer();
      U32 count = p->getBufferSize() / 2;
      while(count--)
      {
         if(max < *ptr)
            max = *ptr;
         ptr++;
      }

      logprintf("queued buffer %d - %d max %d len\n", buffer, max, mInitialBuffer->getBufferSize());
      alBufferData(buffer, AL_FORMAT_MONO16, mInitialBuffer->getBuffer(),
            mInitialBuffer->getBufferSize(), 8000);
      alSourceQueueBuffers(source, 1, &buffer);
   }
}

void SFXObject::playOnSource()
{
   ALuint source = gSources[mSourceIndex];
   alSourceStop(source);
   unqueueBuffers(mSourceIndex);

   if(mInitialBuffer.isValid())
   {
      ALuint buffer = gVoiceFreeBuffers.first();
      gVoiceFreeBuffers.pop_front();

      alBufferData(buffer, AL_FORMAT_MONO16, mInitialBuffer->getBuffer(),
            mInitialBuffer->getBufferSize(), 8000);
      alSourceQueueBuffers(source, 1, &buffer);
      gQueuedBuffers[mSourceIndex] = true;
   }
   else
   {
      alSourcei(source, AL_BUFFER, gBuffers[mSFXIndex]);
      gQueuedBuffers[mSourceIndex] = false;
   }

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
   gVoiceFreeBuffers.setSize(32);
   alGenBuffers(32, gVoiceFreeBuffers.address());

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
      {
         logprintf("Failure (1) loading sound file '%s'", gSFXProfiles[i].fileName);
         return;
      }
      alBufferData(gBuffers[i], format, data, size, freq);
      alutUnloadWAV(format, data, size, freq);
      if(alGetError() != AL_NO_ERROR)
      {
         logprintf("Failure (2) loading sound file '%s'", gSFXProfiles[i].fileName);
         return;
      }
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
      unqueueBuffers(s->mSourceIndex);

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
