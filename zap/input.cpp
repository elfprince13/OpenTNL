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

namespace Zap
{

static bool updateMoveInternal( Move *theMove, U32 &buttonMask )
{
   F32 axes[MaxJoystickAxes];
   if(!ReadJoystick(axes, buttonMask))
      return false;

   // All axes return -1 to 1
   // now we map the controls:

   F32 controls[4];
   controls[0] = axes[0];
   controls[1] = axes[1];
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
   if(OptionsMenuUserInterface::joystickType == XBoxController)
   {
      U32 retMask = 0;
      if(buttonMask & BIT(2))
         retMask |= BIT(0);
      if(buttonMask & BIT(0))
         retMask |= BIT(1);
      if(buttonMask & BIT(5))
         retMask |= BIT(2);
      if(buttonMask & BIT(3))
         retMask |= BIT(3);
      if(buttonMask & BIT(1))
         retMask |= BIT(4);
      if(buttonMask & BIT(4))
         retMask |= BIT(5);
      if(buttonMask & BIT(10))
         retMask |= BIT(6);
      if(buttonMask & BIT(11))
         retMask |= BIT(7);
      buttonMask = retMask;
   }
   return true;
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

   for(U32 i = 0; i < MaxJoystickButtons; i++)
   {
      U32 mask = 1 << i;
      if(buttonsPressed & ~lastButtonsPressed & mask)
         UserInterface::current->onControllerButtonDown(i);
      else if(~buttonsPressed & lastButtonsPressed & mask)
         UserInterface::current->onControllerButtonUp(i);
   }
   lastButtonsPressed = buttonsPressed;
}


};