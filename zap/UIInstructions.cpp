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
#include "UIGame.h"
#include "game.h"
#include "glutInclude.h"
#include "gameObjectRender.h"
#include "ship.h"

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
   if(gClientGame->isConnectedToServer())
   {
      gGameUserInterface.render();
      glEnable(GL_BLEND);
      glBegin(GL_POLYGON);
      glColor4f(0, 0, 0, 0.7);
      glVertex2f(0,0);
      glVertex2f(canvasWidth, 0);
      glVertex2f(canvasWidth, canvasHeight);
      glVertex2f(0, canvasHeight);
      glEnd();
      glDisable(GL_BLEND);
   }

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

static ControlString gControls[] = {
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

static const char *loadoutInstructions[] = {
   "Players can outfit their ships with 3 weapons and 2 modules.",
   "Pressing the loadout selection menu key brings up a menu that",
   "allows the player to choose the next loadout for his or her ship.",
   "This loadout will not be active on the ship until the player",
   "flies over a Loadout Zone object.",
   "",
   "The available modules and their functions are described below:",
   NULL,
};

static const char *moduleDescriptions[] = {
   "Boost - Gives the ship a boost of speed.",
   "Shield - Reflects incoming projectiles.",
   "Repair - Repairs self and nearby objects that are damaged.",
   "Sensor - Boosts the screen visible distance of the player.",
   "Cloak - Turns the ship invisible.",
};

void InstructionsUserInterface::renderPage2()
{
   S32 y = 75;
   glColor3f(1,1,1);
   for(S32 i = 0; loadoutInstructions[i]; i++)
   {
      drawCenteredString(y, 20, loadoutInstructions[i]);
      y += 26;
   }

   y += 30;
   for(S32 i = 0; i < 5; i++)
   {
      glColor3f(1,1,1);
      drawString(120, y, 20, moduleDescriptions[i]);
      glPushMatrix();
      glTranslatef(60, y + 10, 0);
      glScalef(0.7, 0.7, 1);
      glRotatef(-90, 0, 0, 1);
      static F32 thrusts[4] =  { 1, 0, 0, 0 };
      static F32 thrustsBoost[4] =  { 1.3, 0, 0, 0 };

      switch(i)
      {
         case 0:
            renderShip(Color(0,0,1), 1, thrustsBoost, 1, Ship::CollisionRadius, false, false);
            glBegin(GL_LINES);
            glColor3f(1,1,0);
            glVertex2f(-20, -17);
            glColor3f(0,0,0);
            glVertex2f(-20, -50);
            glColor3f(1,1,0);
            glVertex2f(20, -17);
            glColor3f(0,0,0);
            glVertex2f(20, -50);
            glEnd();
            break;
         case 1:
            renderShip(Color(0,0,1), 1, thrusts, 1, Ship::CollisionRadius, false, true);
            break;
         case 2:
            {
               F32 health = (gClientGame->getCurrentTime() & 0x7FF) * 0.0005f;

               renderShip(Color(0,0,1), 1, thrusts, health, Ship::CollisionRadius, false, false);
               glLineWidth(3);
               glColor3f(1,0,0);
               drawCircle(Point(0,0), Ship::RepairDisplayRadius);
               glLineWidth(1);
            }
            break;
         case 3:
            {
               renderShip(Color(0,0,1), 1, thrusts, 1, Ship::CollisionRadius, false, false);
               F32 radius = (gClientGame->getCurrentTime() & 0x1FF) * 0.002;
               drawCircle(Point(), radius * Ship::CollisionRadius + 4);
            }
            break;
         case 4:
            {
               U32 ct = gClientGame->getCurrentTime();
               F32 frac = ct & 0x3FF;
               F32 alpha;
               if((ct & 0x400) != 0)
                  alpha = frac * 0.001;
               else
                  alpha = 1 - (frac * 0.001);
               renderShip(Color(0,0,1), alpha, thrusts, 1, Ship::CollisionRadius, false, false);
            }
            break;
      }

      glPopMatrix();
      y += 60;
   }
}

void InstructionsUserInterface::nextPage()
{
   mCurPage++;
   if(mCurPage > NumPages)
   {
      if(gClientGame->isConnectedToServer())
         gGameUserInterface.activate();
      else
         gMainMenuUserInterface.activate();
   }
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
         if(gClientGame->isConnectedToServer())
            gGameUserInterface.activate();
         else
            gMainMenuUserInterface.activate();
         break;
   }
}

};