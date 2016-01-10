//-----------------------------------------------------------------------------------
//
//   Torque Network Library
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

#include "tnlTypes.h"
#include "tnl.h"
#include "tnlJournal.h"

#include <string.h>
#if defined (TNL_OS_XBOX)
#include <xtl.h>

#elif defined (TNL_OS_WIN32)
#include <windows.h>

#include <malloc.h>

#else

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#endif

#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include "tnlRandom.h"
#include "tnlLog.h"

namespace TNL {

#if defined (TNL_OS_XBOX)

void Platform::outputDebugString(const char *string)
{
   OutputDebugString(string);
}

void Platform::debugBreak()
{
   DebugBreak();
}

void Platform::forceQuit()
{
   logprintf("-Force Quit-");
   // Reboot!
   LD_LAUNCH_DASHBOARD LaunchData = { XLD_LAUNCH_DASHBOARD_MAIN_MENU };
   XLaunchNewImage( NULL, (LAUNCH_DATA*)&LaunchData );
}

U32 Platform::getRealMilliseconds()
{
   U32 tickCount;
   TNL_JOURNAL_READ_BLOCK ( getRealMilliseconds,
      TNL_JOURNAL_READ( (&tickCount) );
      return tickCount;
   )

   tickCount = GetTickCount();

   TNL_JOURNAL_WRITE_BLOCK ( getRealMilliseconds,
      TNL_JOURNAL_WRITE( (tickCount) );
   )
   return tickCount;
}


//--------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
   TNLLogMessageV(LogPlatform, ("AlertOK: %s - %s", message, windowTitle));
   return;
}

//--------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   return MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
   TNLLogMessageV(LogPlatform, ("AlertOKCancel: %s - %s", message, windowTitle));
   return false;
}

//--------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   return (MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
   TNLLogMessageV(LogPlatform, ("AlertRetry: %s - %s", message, windowTitle));
   return false;
}


class WinTimer
{
   private:
      F64 mPeriod;
      bool mUsingPerfCounter;
   public:
      WinTimer()
      {
         S64 frequency;
         mUsingPerfCounter = QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
         mPeriod = 1000.0f / F64(frequency);
      }
      S64 getCurrentTime()
      {
         if(mUsingPerfCounter)
         {
            S64 value;
            QueryPerformanceCounter( (LARGE_INTEGER *) &value);
            return value;
         }
         else
         {
            return GetTickCount();
         }
      }
      F64 convertToMS(S64 delta)
      {
         if(mUsingPerfCounter)
            return mPeriod * F64(delta);
         else
            return F64(delta);
      }
};

static WinTimer gTimer;

S64 Platform::getHighPrecisionTimerValue()
{
   return gTimer.getCurrentTime();
}

F64 Platform::getHighPrecisionMilliseconds(S64 timerDelta)
{
   return gTimer.convertToMS(timerDelta);
}

void Platform::sleep(U32 msCount)
{
	// no need to sleep on the xbox...
}

#elif defined (TNL_OS_WIN32)

bool Platform::checkHeap()
{
#ifdef TNL_COMPILER_VISUALC
   return _heapchk() == _HEAPOK;
#else
   return true;
#endif
}

void Platform::outputDebugString(const char *string)
{
   OutputDebugString(string);
}

void Platform::debugBreak()
{
   DebugBreak();
}

void Platform::forceQuit()
{
   ExitProcess(1);
}

U32 Platform::getRealMilliseconds()
{
   U32 tickCount;
   TNL_JOURNAL_READ_BLOCK ( getRealMilliseconds,
      TNL_JOURNAL_READ( (&tickCount) );
      return tickCount;
   )

   tickCount = GetTickCount();

   TNL_JOURNAL_WRITE_BLOCK ( getRealMilliseconds,
      TNL_JOURNAL_WRITE( (tickCount) );
   )
   return tickCount;
}

class WinTimer
{
   private:
      F64 mPeriod;
      bool mUsingPerfCounter;
   public:
      WinTimer()
      {
         S64 frequency;
         mUsingPerfCounter = QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
         mPeriod = 1000.0f / F64(frequency);
      }
      S64 getCurrentTime()
      {
         if(mUsingPerfCounter)
         {
            S64 value;
            QueryPerformanceCounter( (LARGE_INTEGER *) &value);
            return value;
         }
         else
         {
            return GetTickCount();
         }
      }
      F64 convertToMS(S64 delta)
      {
         if(mUsingPerfCounter)
            return mPeriod * F64(delta);
         else
            return F64(delta);
      }
};

static WinTimer gTimer;

S64 Platform::getHighPrecisionTimerValue()
{
   S64 currentTime;
   TNL_JOURNAL_READ_BLOCK ( getHighPrecisionTimerValue,
      TNL_JOURNAL_READ( (&currentTime) );
      return currentTime;
   )

   currentTime = gTimer.getCurrentTime();

   TNL_JOURNAL_WRITE_BLOCK ( getHighPrecisionTimerValue,
      TNL_JOURNAL_WRITE( (currentTime) );
   )

   return currentTime;
}

F64 Platform::getHighPrecisionMilliseconds(S64 timerDelta)
{
   F64 timerValue;
   TNL_JOURNAL_READ_BLOCK ( getHighPrecisionMilliseconds,
      TNL_JOURNAL_READ( (&timerValue) );
      return timerValue;
   )

   timerValue = gTimer.convertToMS(timerDelta);

   TNL_JOURNAL_WRITE_BLOCK ( getHighPrecisionMilliseconds,
      TNL_JOURNAL_WRITE( (timerValue) );
   )

   return timerValue;
}

void Platform::sleep(U32 msCount)
{
   Sleep(msCount);
}

//--------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
   ShowCursor(true);
   MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
}

//--------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
   ShowCursor(true);
   return MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
}

//--------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
   ShowCursor(true);
   return (MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
}

#else // osx and linux

void Platform::debugBreak()
{
   kill(getpid(), SIGTRAP);
}

void Platform::outputDebugString(const char *string)
{
   //printf("%s", string);
}

void Platform::forceQuit()
{
   debugBreak();
   exit(1);
}


size_t x86UNIXGetTickCount();
//--------------------------------------

U32 Platform::getRealMilliseconds()
{
   return x86UNIXGetTickCount();
}

static bool   sg_initialized = false;
static size_t sg_secsOffset  = 0;

size_t x86UNIXGetTickCount()
{
   // TODO: What happens when crossing a day boundary?
   //
   timeval t;

   if (sg_initialized == false) {
      sg_initialized = true;

      ::gettimeofday(&t, NULL);
      sg_secsOffset = t.tv_sec;
   }

   ::gettimeofday(&t, NULL);

   size_t secs  = t.tv_sec - sg_secsOffset;
   U32 uSecs = t.tv_usec;

   // Make granularity 1 ms
   return (secs * 1000) + (uSecs / 1000);
}

class UnixTimer
{
   public:
      UnixTimer()
      {
      }
      S64 getCurrentTime()
      {
         return x86UNIXGetTickCount();
      }
      F64 convertToMS(S64 delta)
      {
         return F64(delta);
      }
};

static UnixTimer gTimer;

S64 Platform::getHighPrecisionTimerValue()
{
   return gTimer.getCurrentTime();
}

F64 Platform::getHighPrecisionMilliseconds(S64 timerDelta)
{
   return gTimer.convertToMS(timerDelta);
}

void Platform::sleep(U32 msCount)
{
   usleep(msCount * 1000);
}

//--------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OK);
   TNLLogMessageV(LogPlatform, ("AlertOK: %s - %s", message, windowTitle));
   return;
}

//--------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   return MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_OKCANCEL) == IDOK;
   TNLLogMessageV(LogPlatform, ("AlertOKCancel: %s - %s", message, windowTitle));
   return false;
}

//--------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
//   ShowCursor(true);
//   return (MessageBox(NULL, message, windowTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_RETRYCANCEL) == IDRETRY);
   TNLLogMessageV(LogPlatform, ("AlertRetry: %s - %s", message, windowTitle));
   return false;
}

#endif

/*
char *strdup(const char *src)
{
   char *buffer = (char *) malloc(strlen(src) + 1);
   strcpy(buffer, src);
   return buffer;
}*/
bool atob(const char *str)
{
   return !stricmp(str, "true") || atof(str);
}

S32 dSprintf(char *buffer, U32 bufferSize, const char *format, ...)
{
   va_list args;
   va_start(args, format);
#ifdef TNL_COMPILER_VISUALC
   S32 len = _vsnprintf(buffer, bufferSize, format, args);
#else
   S32 len = vsnprintf(buffer, bufferSize, format, args);
#endif
   return (len);
}


S32 dVsprintf(char *buffer, size_t bufferSize, const char *format, va_list arglist)
{
#ifdef TNL_COMPILER_VISUALC
   S32 len = _vsnprintf(buffer, bufferSize, format, (va_list) arglist);
#else
   S32 len = vsnprintf(buffer, bufferSize, format, arglist);
#endif
   return len;
}

};


#if defined (__GNUC__)

int stricmp(const char *str1, const char *str2)
{
   while(toupper(*str1) == toupper(*str2) && *str1)
   {
      str1++;
      str2++;
   }
   return (toupper(*str1) > toupper(*str2)) ? 1 : ((toupper(*str1) < toupper(*str2)) ? -1 : 0);
}

int strnicmp(const char *str1, const char *str2, size_t len)
{
   for(size_t i = 0; i < len; i++)
   {
      if(toupper(str1[i]) == toupper(str2[i]))
         continue;
      return (toupper(str1[i]) > toupper(str2[i])) ? 1 : ((toupper(str1[i]) < toupper(str2[i])) ? -1 : 0);
   }
   return 0;
}

#endif


