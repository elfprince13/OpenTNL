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

#include "input.h"
#include "move.h"
#include "UIMenus.h"
#include "point.h"
#include "tnlJournal.h"
#include <math.h>
#include "glutInclude.h"

namespace Zap
{

static bool updateMoveInternal( Move *theMove, U32 &buttonMask )
{
   F32 axes[MaxJoystickAxes];
   static F32 minValues[2] = { - 0.5, -0.5 };
   static F32 maxValues[2] = { 0.5, 0.5 };

   if(!ReadJoystick(axes, buttonMask))
      return false;

   // All axes return -1 to 1
   // now we map the controls:

   F32 controls[4];
   controls[0] = axes[0];
   controls[1] = axes[1];

   if(controls[0] < minValues[0])
      minValues[0] = controls[0];
   if(controls[0] > maxValues[0])
      maxValues[0] = controls[0];
   if(controls[1] < minValues[1])
      minValues[1] = controls[1];
   if(controls[1] > maxValues[1])
      maxValues[1] = controls[1];

   if(controls[0] < 0)
      controls[0] = - (controls[0] / minValues[0]);
   else if(controls[0] > 0)
      controls[0] = (controls[0] / maxValues[0]);

   if(controls[1] < 0)
      controls[1] = - (controls[1] / minValues[1]);
   else if(controls[1] > 0)
      controls[1] = (controls[1] / maxValues[1]);

   // xbox control inputs are in a circle, not a square, which makes
   // diagonal movement inputs "slower"
   if(OptionsMenuUserInterface::joystickType == XBoxController ||
      OptionsMenuUserInterface::joystickType == XBoxControllerOnXBox)
   {
      Point dir(controls[0], controls[1]);
      F32 absX = fabs(dir.x);
      F32 absY = fabs(dir.y);

      // push out to the edge of the square (-1,-1 -> 1,1 )

      F32 dirLen = dir.len() * 1.25;
      if(dirLen > 1)
         dirLen = 1;

      if(absX > absY)
         dir *= F32(dirLen / absX);
      else
         dir *= F32(dirLen / absY);
      controls[0] = dir.x;
      controls[1] = dir.y;
   }
   switch(OptionsMenuUserInterface::joystickType)
   {
      case LogitechWingman:
         controls[2] = axes[5];
         controls[3] = axes[6];
         break;
      case SaitekDualAnalog:
         controls[2] = axes[5];
         controls[3] = axes[2];
         break;
      case PS2DualShock:
         controls[2] = axes[2];
         controls[3] = axes[5];
         break;
      case XBoxController:
      case XBoxControllerOnXBox:
         controls[2] = axes[3];
         controls[3] = axes[4];
         break;
   }

   for(U32 i = 0; i < 4; i++)
   {
      F32 deadZone = i < 2 ? 0.25f : 0.03125f;
      if(controls[i] < -deadZone)
         controls[i] = -(-controls[i] - deadZone) / F32(1 - deadZone);
      else if(controls[i] > deadZone)
         controls[i] = (controls[i] - deadZone) / F32(1 - deadZone);
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
   F32 plen = p.len();
   if(plen > 0.3)
   {
      theMove->angle = atan2(p.y, p.x);
      theMove->fire = (plen > 0.5);
   }
   else
      theMove->fire = false;

   
   // Remap crazy xbox inputs...
   if(OptionsMenuUserInterface::joystickType == LogitechWingman)
   {
      if(buttonMask & (1 << 8))
      {
         buttonMask |= ControllerButtonBack;
         buttonMask &= ~(1 << 8);
      }
   }
   else if(OptionsMenuUserInterface::joystickType == SaitekDualAnalog)
   {
      static U32 retMasks[12] = {
         ControllerButton1,
         ControllerButton2,
         ControllerButton3,
         ControllerButton4,
         ControllerButton5,
         ControllerButton6,
         ControllerButtonLeftTrigger,
         ControllerButtonRightTrigger,
         0,
         ControllerButton1,
         ControllerButtonBack,
         0,
      };
      U32 retMask = 0;
      for(S32 i = 0; i < 12; i++)
         if(buttonMask & (1 << i))
            retMask |= retMasks[i];
      buttonMask = retMask;
   }
   else if(OptionsMenuUserInterface::joystickType == PS2DualShock)
   {
      static U32 retMasks[12] = {
         ControllerButton4,
         ControllerButton2,
         ControllerButton1,
         ControllerButton3,
         ControllerButton5,
         ControllerButton6,
         ControllerButtonLeftTrigger,
         ControllerButtonRightTrigger,
         ControllerButtonBack,
         0,
         ControllerButton1,
         ControllerButtonStart,
      };
      U32 retMask = 0;
      for(S32 i = 0; i < 12; i++)
         if(buttonMask & (1 << i))
            retMask |= retMasks[i];
      buttonMask = retMask;
   }
   else if(OptionsMenuUserInterface::joystickType == XBoxController)
   {
      static U32 retMasks[12] = {
         ControllerButton1,
         ControllerButton2,
         ControllerButton3,
         ControllerButton4,
         ControllerButton6,
         ControllerButton5,
         ControllerButtonStart,
         ControllerButtonBack,
         0,
         ControllerButton1,
         ControllerButtonLeftTrigger,
         ControllerButtonRightTrigger,
      };
      U32 retMask = 0;
      for(S32 i = 0; i < 12; i++)
         if(buttonMask & (1 << i))
            retMask |= retMasks[i];
      buttonMask = retMask;
   }
   return true;
}

S32 autodetectJoystickType()
{
   S32 ret = -1;
   TNL_JOURNAL_READ_BLOCK(JoystickAutodetect, 
      TNL_JOURNAL_READ((&ret));
      return ret;
      )
   const char *joystickName = GetJoystickName();
   if(!strncmp(joystickName, "WingMan", 7))
      ret = LogitechWingman;
   else if(!strcmp(joystickName, "XBoxOnXBox"))
      ret = XBoxControllerOnXBox;
   else if(strstr(joystickName, "XBox"))
      ret = XBoxController;
   else if(!strcmp(joystickName, "4 axis 16 button joystick"))
      ret = PS2DualShock;
   else if(strstr(joystickName, "P880"))
      ret = SaitekDualAnalog;
   TNL_JOURNAL_WRITE_BLOCK(JoystickAutodetect,
      TNL_JOURNAL_WRITE((ret));
   )
   return ret;
}

static bool updateMoveJournaled( Move *theMove, U32 &buttonMask )
{
   TNL_JOURNAL_READ_BLOCK(JoystickUpdate,
      BitStream *readStream = Journal::getReadStream();
      if(!readStream->readFlag())
         return false;

      Move aMove;
      aMove.unpack(readStream, false);
      *theMove = aMove;
      buttonMask = readStream->readInt(MaxJoystickButtons);
      return true;
   )

   bool ret = updateMoveInternal(theMove, buttonMask);

   TNL_JOURNAL_WRITE_BLOCK(JoystickUpdate,
      BitStream *writeStream = Journal::getWriteStream();
      if(writeStream->writeFlag(ret))
      {
         Move dummy;
         theMove->pack(writeStream, &dummy, false);
         writeStream->writeInt(buttonMask, MaxJoystickButtons);
      }
   )
   return ret;
}



void JoystickUpdateMove(Move *theMove)
{
   static U32 lastButtonsPressed = 0;
   U32 buttonsPressed;
   if(!updateMoveJournaled(theMove, buttonsPressed))
      return;

   U32 buttonDown = buttonsPressed & ~lastButtonsPressed;
   U32 buttonUp = ~buttonsPressed & lastButtonsPressed;
   lastButtonsPressed = buttonsPressed;

   for(U32 i = 0; i < ControllerGameButtonCount; i++)
   {
      U32 mask = 1 << i;
      if(buttonDown & mask)
         UserInterface::current->onControllerButtonDown(i);
      else if(buttonUp & mask)
         UserInterface::current->onControllerButtonUp(i);
   }
   if(buttonDown & ControllerButtonStart)
      UserInterface::current->onKeyDown('\r');
   if(buttonDown & ControllerButtonBack)
      UserInterface::current->onKeyDown(27);
   if(buttonDown & ControllerButtonDPadUp)
      UserInterface::current->onSpecialKeyDown(GLUT_KEY_UP);
   if(buttonDown & ControllerButtonDPadDown)
      UserInterface::current->onSpecialKeyDown(GLUT_KEY_DOWN);
   if(buttonDown & ControllerButtonDPadLeft)
      UserInterface::current->onSpecialKeyDown(GLUT_KEY_LEFT);
   if(buttonDown & ControllerButtonDPadRight)
      UserInterface::current->onSpecialKeyDown(GLUT_KEY_RIGHT);

   if(buttonUp & ControllerButtonStart)
      UserInterface::current->onKeyUp('\r');
   if(buttonUp & ControllerButtonBack)
      UserInterface::current->onKeyUp(27);
   if(buttonUp & ControllerButtonDPadUp)
      UserInterface::current->onSpecialKeyUp(GLUT_KEY_UP);
   if(buttonUp & ControllerButtonDPadDown)
      UserInterface::current->onSpecialKeyUp(GLUT_KEY_DOWN);
   if(buttonUp & ControllerButtonDPadLeft)
      UserInterface::current->onSpecialKeyUp(GLUT_KEY_LEFT);
   if(buttonUp & ControllerButtonDPadRight)
      UserInterface::current->onSpecialKeyUp(GLUT_KEY_RIGHT);
}


};