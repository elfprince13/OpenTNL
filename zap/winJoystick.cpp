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

#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>

#include "gameConnection.h"
#include "point.h"
#include "UI.h"
#include "UIMenus.h"
#include "tnlJournal.h"

namespace Zap
{

extern const char *gWindowTitle;
extern U32 gJoystickType;

void checkMousePos(S32 maxdx, S32 maxdy)
{
   if(OptionsMenuUserInterface::controlsRelative)
      return;

   char windowName[256];

   HWND theWindow = GetForegroundWindow();
   GetWindowText(theWindow, windowName, sizeof(windowName));

   if(strcmp(windowName, gWindowTitle))
      return;

   RECT r;

   GetWindowRect(theWindow, &r);
   POINT cp;
   GetCursorPos(&cp);

   // Check our position in the window; if we're close to the top (within 32px) then ignore
   // same if we're within 5px of the edges. That way we can avoid going crazy when
   // people try to resize/drag the window.
   if(cp.x - r.top < 32)   return;
   if(cp.y - r.left < 5)   return;
   if(r.right - cp.y < 5)  return;
   if(r.bottom - cp.x < 5) return;

   S32 centerX = (r.right + r.left) >> 1;
   S32 centerY = (r.bottom + r.top) >> 1;

   cp.x -= centerX;
   cp.y -= centerY;

   if(cp.x > maxdx)
      cp.x = maxdx;
   if(cp.y > maxdy)
      cp.y = maxdy;
   if(cp.x < -maxdx)
      cp.x = -maxdx;
   if(cp.y < -maxdy)
      cp.y = -maxdy;

   SetCursorPos(int(centerX + cp.x), int(centerY + cp.y));
}

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

LPDIRECTINPUT8 gDirectInput = NULL;
LPDIRECTINPUTDEVICE8 gJoystick = NULL;

void InitController()
{
   if(FAILED(DirectInput8Create ( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
         IID_IDirectInput8, (VOID**)&gDirectInput, NULL ) ) )
      return;

   if(FAILED(gDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback,
         NULL, DIEDFL_ATTACHEDONLY ) ) )
      return;

   if(!gJoystick)
      return;

   if( FAILED(gJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
      return;

   // since we passed in a NULL window, we don't bother setting the cooperative level.
}


BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
   // Obtain an interface to the enumerated joystick.
   if(FAILED(gDirectInput->CreateDevice( pdidInstance->guidInstance, &gJoystick, NULL )))
      return DIENUM_CONTINUE;
   
   return DIENUM_STOP;
}


static bool updateMoveInternal( Move *theMove, bool buttonPressed[12] )
{
   // mark: it's ok
   // mark: it's called "winJoystick"
   // mark: it's supposed to be gross.

   DIJOYSTATE2 js;       // DInput joystick state 

   if(!gJoystick)
      return false;

   if(FAILED(gJoystick->Poll() ) )
   {
      HRESULT hr;
      hr = gJoystick->Acquire();
      
      while( hr == DIERR_INPUTLOST ) 
         hr = gJoystick->Acquire();
      return false; 
   }

   // Get the input's device state
   if(FAILED(gJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
      return false; // The device should have been acquired during the Poll()

   S32 x = js.lX;
   F32 deadZone = 8192.0f;

   F32 controls[4];
   controls[0] = F32( js.lX ) - 32768.0f;
   controls[1] = F32( js.lY ) - 32768.0f;

   if(gJoystickType == 0)
   {
      controls[2] = F32( js.lRz ) - 32768.0f;
      controls[3] = F32( js.rglSlider[0] ) - 32768.0f;
   }
   else if(gJoystickType == 1)
   {
      controls[3] = F32( js.lZ ) - 32768.0f;      
      controls[2] = F32( js.lRz ) - 32768.0f;
   }
   else if(gJoystickType == 2)
   {
      controls[2] = F32( js.lZ ) - 32768.0f;
      controls[3] = F32( js.lRz ) - 32768.0f;
   }
   else if(gJoystickType == 3)
   {
      controls[2] = F32( js.lRx ) - 32768.0f;
      controls[3] = F32( js.lRy ) - 32768.0f;
   }

   for(U32 i = 0; i < 4; i++)
   {
      if(controls[i] < -deadZone)
         controls[i] = -(-controls[i] - deadZone) / F32(32768.0f - deadZone);
      else if(controls[i] > deadZone)
         controls[i] = (controls[i] - deadZone) / F32(32768.0f - deadZone);
      else
         controls[i] = 0;
   }
   if(controls[0] < 0)
   {
      theMove->left = -controls[0];
      theMove->right = 0;
   }
   else
   {
      theMove->left = 0;
      theMove->right = controls[0];
   }

   if(controls[1] < 0)
   {
      theMove->up = -controls[1];
      theMove->down = 0;
   }
   else
   {
      theMove->down = controls[1];
      theMove->up = 0;
   }

   Point p(controls[2], controls[3]);
   if(p.len() > 0.1)
   {
      theMove->angle = atan2(p.x, p.y);
      theMove->fire = true;
   }
   else
      theMove->fire = false;

   // check the state of the buttons:
   for( U32 i = 0; i < 12; i++ )
      buttonPressed[i] = (js.rgbButtons[i] & 0x80) != 0;

   // Remap crazy xbox inputs...
   if(gJoystickType == 3)
   {
      bool b[12];

      // Get the first six buttons...
      b[0] = buttonPressed[2];
      b[1] = buttonPressed[0];
      b[2] = buttonPressed[5];

      b[3] = buttonPressed[3];
      b[4] = buttonPressed[1];
      b[5] = buttonPressed[4];

      // And the shoulder buttons...
      b[6] = buttonPressed[10];
      b[7] = buttonPressed[11];

      // Fill in rest with false...
      for( U32 i=8; i<12; i++)
         b[i] = false;

      // Now put it all back into the array (this is O(1), really!)
      for( U32 i=0; i<12; i++)
         buttonPressed[i] = b[i];
   }

   

   return true;
}

static bool updateMoveInternalJournaled( Move *theMove, bool buttonPressed[12] )
{
   TNL_JOURNAL_READ_BLOCK(JoystickUpdate,
      BitStream *readStream = Journal::getReadStream();
      if(!readStream->readFlag())
         return false;

      Move aMove;
      aMove.unpack(readStream, false);
      *theMove = aMove;
      for(U32 i = 0; i < 12; i++)
         buttonPressed[i] = readStream->readFlag();
      return true;
   )

   bool ret = updateMoveInternal(theMove, buttonPressed);

   TNL_JOURNAL_WRITE_BLOCK(JoystickUpdate,
      BitStream *writeStream = Journal::getWriteStream();
      if(writeStream->writeFlag(ret))
      {
         Move dummy;
         theMove->pack(writeStream, &dummy, false);
         for(U32 i = 0;i < 12; i++)
            writeStream->writeFlag(buttonPressed[i]);
      }
   )
   return ret;
}

void JoystickUpdateMove( Move *theMove )
{
   static bool buttonDown[12] = { 0, };
   bool buttonPressed[12];

   if(!updateMoveInternalJournaled( theMove, buttonPressed ))
      return;

   for(U32 i = 0; i < 12; i++)
   {
      if(buttonPressed[i] && !buttonDown[i])
         UserInterface::current->onControllerButtonDown(i);
      else if(!buttonPressed[i] && buttonDown[i])
         UserInterface::current->onControllerButtonUp(i);
      buttonDown[i] = buttonPressed[i];
   }
}

void FreeDirectInput()
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( gJoystick ) 
        gJoystick->Unacquire();
    
    // Release any DirectInput objects.
    if(gJoystick)
      gJoystick->Release();
    if(gDirectInput)
      gDirectInput->Release();
}

};
