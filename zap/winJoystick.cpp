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

namespace Zap
{

extern const char *gWindowTitle;
extern U32 gJoystickType;

void checkMousePos(S32 maxdx, S32 maxdy)
{
   char windowName[256];

   HWND theWindow = GetForegroundWindow();
   GetWindowText(theWindow, windowName, sizeof(windowName));

   if(strcmp(windowName, gWindowTitle))
      return;

   RECT r;

   GetWindowRect(theWindow, &r);
   POINT cp;
   GetCursorPos(&cp);

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

void JoystickUpdateMove( Move *theMove )
{
    DIJOYSTATE2 js;           // DInput joystick state 

    if(!gJoystick) 
        return;

    if(FAILED(gJoystick->Poll() ) )
    {
       HRESULT hr;
       hr = gJoystick->Acquire();
        
       while( hr == DIERR_INPUTLOST ) 
          hr = gJoystick->Acquire();
       return; 
    }

    // Get the input's device state
    if(FAILED(gJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
        return; // The device should have been acquired during the Poll()

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

   static bool buttonDown[12] = { 0, };
   // check the state of the buttons:
   for( U32 i = 0; i < 12; i++ )
   {
      bool buttonPressed = (js.rgbButtons[i] & 0x80) != 0;
      if(buttonPressed && !buttonDown[i])
         UserInterface::current->onControllerButtonDown(i);
      else if(!buttonPressed && buttonDown[i])
         UserInterface::current->onControllerButtonUp(i);
      buttonDown[i] = buttonPressed;
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
