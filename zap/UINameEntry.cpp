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

#include "UINameEntry.h"
#include "UIMenus.h"
#include "game.h"

#include "glutInclude.h"

namespace Zap
{

NameEntryUserInterface gNameEntryUserInterface;

void NameEntryUserInterface::render()
{
   glViewport(0, 0, windowWidth, windowHeight);

   glClearColor(0, 0, 0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glColor3f(1,1,1);

   U32 y = (canvasHeight / 2) - 20;

   drawCenteredString(y, 20, title);

   y += 45;
   drawCenteredString(y, 30, buffer);

   U32 width = getStringWidth(30, buffer);
   U32 x = (canvasWidth - width) / 2;

   if(blink)
      drawString(x + getStringWidth(30, buffer, cursorPos), y, 30, "_");
}

void NameEntryUserInterface::idle(U32 t)
{
   if(mBlinkTimer.update(t))
   {
      mBlinkTimer.reset(BlinkTime);
      blink = !blink;
   }
}

void NameEntryUserInterface::onKeyDown(U32 key)
{
   if(key == '\r')
      onAccept();
   else if(key == 8)
   {
      // backspace key
      if(cursorPos > 0)
      {
         cursorPos--;
         for(U32 i = cursorPos; buffer[i]; i++)
            buffer[i] = buffer[i+1];
      }
   }
   else if(key == 27)
   {
      onEscape();
   }
   else
   {
      for(U32 i = MaxNameLen - 1; i > cursorPos; i--)
         buffer[i] = buffer[i-1];
      buffer[cursorPos] = key;
      if(cursorPos < MaxNameLen-1)
         cursorPos++;
   }
}

void NameEntryUserInterface::onKeyUp(U32 key)
{

}

void NameEntryUserInterface::onEscape()
{
}

void NameEntryUserInterface::setText(const char *text)
{
   if(strlen(text) > MaxNameLen)
   {
      strncpy(buffer, text, MaxNameLen);
      buffer[MaxNameLen] = 0;
   }
   else
      strcpy(buffer, text);
}

void NameEntryUserInterface::onAccept()
{
   gMainMenuUserInterface.activate();
}

IPEntryUserInterface gIPEntryUserInterface;

void IPEntryUserInterface::onAccept()
{
   joinGame(Address(getText()), false);
}

void IPEntryUserInterface::onEscape()
{
   gMainMenuUserInterface.activate();
}

};