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

#include "../tnl/tnl.h"
using namespace TNL;

#include "UI.h"
#include <stdarg.h>

#include "glutInclude.h"

namespace Zap
{

F32 UserInterface::canvasWidth = 800;
F32 UserInterface::canvasHeight = 600;
F32 UserInterface::windowWidth = 800;
F32 UserInterface::windowHeight = 600;

UserInterface *UserInterface::current = NULL;

void UserInterface::activate()
{
   current = this;
   onActivate();
}

void UserInterface::onActivate()
{
}

void UserInterface::drawString(S32 x, S32 y, S32 size, const char *string)
{
   F32 scaleFactor = size / 120.0f;
   glPushMatrix();
   glTranslatef(x, y + size, 0);
   glScalef(scaleFactor, -scaleFactor, 1);
   for(S32 i = 0; string[i]; i++)
   	glutStrokeCharacter(GLUT_STROKE_ROMAN, string[i]);
   glPopMatrix();
}

void UserInterface::drawCenteredString(S32 y, S32 size, const char *string)
{
   S32 x = (canvasWidth - getStringWidth(size, string)) / 2;
   drawString(x, y, size, string);
}

U32 UserInterface::getStringWidth(S32 size, const char *string, U32 len)
{
   S32 width = 0;
   if(!len)
      len = strlen(string);
   while(len--)
   {
      width += glutStrokeWidth(GLUT_STROKE_ROMAN, *string);
      string++;
   }
   return (width * size) / 120.0f;

}


void UserInterface::drawStringf(S32 x, S32 y, S32 size, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   char buffer[2048];

   dVsprintf(buffer, sizeof(buffer), format, args);
   drawString(x, y, size, buffer);
}


void UserInterface::render()
{

}

void UserInterface::idle(U32 timeDelta)
{

}

void UserInterface::onMouseDown(S32 x, S32 y)
{

}

void UserInterface::onMouseUp(S32 x, S32 y)
{

}

void UserInterface::onMouseMoved(S32 x, S32 y)
{

}

void UserInterface::onMouseDragged(S32 x, S32 y)
{

}

void UserInterface::onKeyDown(U32 key)
{

}

void UserInterface::onKeyUp(U32 key)
{

}

void UserInterface::onSpecialKeyDown(U32 key)
{

}

void UserInterface::onSpecialKeyUp(U32 key)
{

}

};