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

#include "UIInstructions.h"
#include "UIMenus.h"
#include "game.h"
#include "glutInclude.h"

namespace Zap
{

InstructionsUserInterface gInstructionsUserInterface;

void InstructionsUserInterface::onActivate()
{
   mCurPage = 1;
}

const char *pageHeaders[] = {
   "CONTROLS",
   "LOADOUT SELECTION",
};

void InstructionsUserInterface::render()
{
   glColor3f(1,1,1);
   drawStringf(3, 3, 25, "INSTRUCTIONS - %s", pageHeaders[mCurPage - 1]);
   drawStringf(650, 3, 25, "PAGE %d/%d", mCurPage, NumPages);
   drawCenteredString(571, 20, "LEFT - previous page  RIGHT, SPACE - next page  ESC exits");
   glColor3f(0.7, 0.7, 0.7);
   glBegin(GL_LINES);
   glVertex2f(0, 31);
   glVertex2f(800, 31);
   glVertex2f(0, 569);
   glVertex2f(800, 569);
   glEnd();
   switch(mCurPage)
   {
      case 1:
         renderPage1();
         break;
      case 2:
         renderPage2();
         break;
   }
}

struct ControlString
{
   const char *controlString;
   const char *primaryControl;
};

ControlString gControls[] = {
   { "Move ship up (forward)", "W" },
   { "Move ship down (backward)", "S" },
   { "Move ship left (strafe left)", "A" },
   { "Move ship right (strafe right)", "D" },
   { "Aim ship", "MOUSE" },
   { "Fire weapon", "MOUSE BUTTON 1" },
   { "Activate primary module", "SPACE" },
   { "Activate secondary module", "SHIFT, MOUSE BUTTON 2" },
   { "Cycle current weapon", "E" },
   { "Open loadout selection menu", "Q" },
   { "Toggle map view", "C" },
   { "Chat to team", "T" },
   { "Chat to everyone", "G" },
   { "Open QuickChat menu", "V" },
   { "Record voice chat", "R" },
   { "Show scoreboard", "TAB" },
   { NULL, NULL },
};


void InstructionsUserInterface::renderPage1()
{
   U32 y = 75;
   U32 col1 = 50;
   U32 col2 = 400;
   glBegin(GL_LINES);
   glVertex2f(col1, y + 26); 
   glVertex2f(750, y + 26);
   glEnd();
   glColor3f(1,1,1);

   drawString(col1, y, 20, "Action");
   drawString(col2, y, 20, "Control");
   y += 28;
   for(S32 i = 0; gControls[i].controlString; i++)
   {
      drawString(col1, y, 20, gControls[i].controlString);
      drawString(col2, y, 20, gControls[i].primaryControl);
      y += 26;
   }
}

void InstructionsUserInterface::renderPage2()
{

}

void InstructionsUserInterface::nextPage()
{
   mCurPage++;
   if(mCurPage > NumPages)
      gMainMenuUserInterface.activate();
}

void InstructionsUserInterface::prevPage()
{
   if(mCurPage > 1)
      mCurPage--;
}

void InstructionsUserInterface::onSpecialKeyDown(U32 key)
{
   switch(key)
   {
      case GLUT_KEY_LEFT:
      case GLUT_KEY_UP:
         prevPage();
         break;
      case GLUT_KEY_RIGHT:
      case GLUT_KEY_DOWN:
         nextPage();
         break;
   }
}

void InstructionsUserInterface::onKeyDown(U32 key)
{
   switch(key)
   {
      case ' ':
         nextPage();
         break;
      case 27:
         gMainMenuUserInterface.activate();
         break;
   }
}

};