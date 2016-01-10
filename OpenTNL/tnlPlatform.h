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

#ifndef _TNL_PLATFORM_H_
#define _TNL_PLATFORM_H_

#ifndef _TNL_TYPES_H_
#include "tnlTypes.h"
#endif

#include <stdarg.h>

namespace TNL {

/// Platform specific functionality is gathered here to enable easier porting.
///
/// If you are embedding TNL in a complex application, you may want to replace
/// these with hooks to your own code - for instance the alerts really should
/// bring up an actual message box; currently they just emit a log item.
namespace Platform
{
   /// Prints a string to the OS specific debug log.
   void outputDebugString(const char *string);

   /// Stops the program in the debugger, if it is being debugged.
   void debugBreak();

   /// Forces the program to exit immediately.
   ///
   /// @note This is probably a bit strong for a networking library. -- BJG
   void forceQuit();

   /// Brings up a dialog window with a message and an "OK" button
   void AlertOK(const char *windowTitle, const char *message);

   /// Brings up a dialog window with the message, and "OK" and "Cancel" buttons
   bool AlertOKCancel(const char *windowTitle, const char *message);

   /// Brings up a dialog window with the message, and "Retry" and "Cancel" buttons
   bool AlertRetry(const char *windowTitle, const char *message);

   /// Elapsed time in milliseconds.
   ///
   /// Usually since last reboot, but it varies from platform to platform. It is
   /// guaranteed to always increase, however, up to the limit of a U32 - about
   /// 7 weeks worth of time. If you are developing a server you want to run for longer
   /// than that, prepared to see wraparounds.
   U32 getRealMilliseconds();

   /// Returns a high-precision time value, in a platform-specific time value
   S64 getHighPrecisionTimerValue();

   /// Converts a high precision timer delta into milliseconds
   F64 getHighPrecisionMilliseconds(S64 timerDelta);

   /// Put the process to sleep for the specified millisecond interva.
   void sleep(U32 msCount);

   /// checks the status of the memory allocation heap
   bool checkHeap();
};


#define TIME_BLOCK(name,block) { S64 st = Platform::getHighPrecisionTimerValue(); {block} S64 delta = Platform::getHighPrecisionTimerValue() - st; F64 ms = Platform::getHighPrecisionMilliseconds(delta); logprintf("Timer: %s Elapsed: %g ms", #name, ms); }

#if defined (TNL_SUPPORTS_VC_INLINE_X86_ASM) || defined (TNL_SUPPORTS_MWERKS_INLINE_X86_ASM)
#define TNL_DEBUGBREAK() { __asm { int 3 }; }
#elif defined(TNL_SUPPORTS_GCC_INLINE_X86_ASM)
#define TNL_DEBUGBREAK() { asm ( "int $3"); }
#else
/// Macro to do in-line debug breaks, used for asserts.  Does inline assembly where appropriate
#define TNL_DEBUGBREAK() Platform::debugBreak();
#endif

#define TNL_CHECK_HEAP() { bool status = TNL::Platform::checkHeap(); if(!status) TNL_DEBUGBREAK(); }

extern bool        atob(const char *str); ///< String to boolean conversion.

/// Printf into string with a buffer size.
///
/// This will print into the specified string until the buffer size is reached.
extern int    dSprintf(char *buffer, U32 bufferSize, const char *format, ...);

/// Vsprintf with buffer size argument.
///
/// This will print into the specified string until the buffer size is reached.
extern int    dVsprintf(char *buffer, U32 bufferSize, const char *format, va_list arglist); ///< compiler independent

inline char dToupper(const char c) { if (c >= char('a') && c <= char('z')) return char(c + 'A' - 'a'); else return c; } ///< Converts an ASCII character to upper case.
inline char dTolower(const char c) { if (c >= char('A') && c <= char('Z')) return char(c - 'A' + 'a'); else return c; } ///< Converts an ASCII character to lower case.

#define QSORT_CALLBACK FN_CDECL

};

#include <string.h>
#include <stdlib.h>

#if defined (__GNUC__)

int stricmp(const char *str1, const char *str2);
int strnicmp(const char *str1, const char *str2, unsigned int len);

#endif


#endif
