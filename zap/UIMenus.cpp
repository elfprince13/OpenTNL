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

#include "UIMenus.h"
#include "UINameEntry.h"
#include "UIGame.h"
#include "UIQueryServers.h"
#include "UICredits.h"
#include "game.h"
#include "gameType.h"
#include "UIEditor.h"

#include "glutInclude.h"

namespace Zap
{

void MenuUserInterface::render()
{
   glViewport(0, 0, windowWidth, windowHeight);

   if(clearBackground)
   {
      glClearColor(0, 0, 0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glColor3f(1,1,1);
   drawCenteredString( 10, 30, menuTitle);
   drawCenteredString( 45, 18, menuSubTitle);
   drawCenteredString( 570, 18, menuFooter);

   U32 yStart = (canvasHeight - menuItems.size() * 45) / 2;
   //glColor3f(0,1,0);

   for(S32 i = 0; i < menuItems.size(); i++)
   {
      U32 y = yStart + i * 45;

      if(selectionIndex == i)
      {
         glColor3f(0,0,0.4);
         glBegin(GL_POLYGON);
         glVertex2f(0, y - 2);
         glVertex2f(800, y - 2);
         glVertex2f(800, y + 25 + 5);
         glVertex2f(0, y + 25 + 5);
         glEnd();
         glColor3f(0,0,1);
         glBegin(GL_LINES);
         glVertex2f(0, y - 2);
         glVertex2f(799, y - 2);
         glVertex2f(799, y + 25 + 5);
         glVertex2f(0, y + 25 + 5);
         glEnd();
      }      
      glColor3f(1,1,1);
      drawCenteredString(y, 25, menuItems[i]);
   }
}

void MenuUserInterface::onSpecialKeyDown(U32 key)
{
   if(key == GLUT_KEY_UP || key == GLUT_KEY_LEFT)
   {
      selectionIndex--;
      if(selectionIndex < 0)
         selectionIndex = menuItems.size() - 1;

      UserInterface::playBoop();
   }
   else if(key == GLUT_KEY_DOWN || key == GLUT_KEY_RIGHT)
   {
      selectionIndex++;
      if(selectionIndex >= menuItems.size())
         selectionIndex = 0;

      UserInterface::playBoop();
   }
}

void MenuUserInterface::onKeyDown(U32 key)
{
   if(key == '\r')
   {
      UserInterface::playBoop();
      processSelection(selectionIndex);
      selectionIndex = 0;
   }
   else if(key == 27)
   {
      UserInterface::playBoop();
      selectionIndex = 0;
      onEscape();
   }
}

void MenuUserInterface::onEscape()
{

}

MainMenuUserInterface gMainMenuUserInterface;

MainMenuUserInterface::MainMenuUserInterface()
{
   dSprintf(titleBuffer, sizeof(titleBuffer), "%s:", ZAP_GAME_STRING);
   menuTitle = titleBuffer;
   menuSubTitle = "A TORQUE NETWORK LIBRARY GAME - WWW.OPENTNL.ORG";
   menuFooter = "(C) 2004 GARAGEGAMES.COM, INC.";

   menuItems.push_back("JOIN LAN/INTERNET GAME");
   menuItems.push_back("JOIN SPECIFIC GAME");
   menuItems.push_back("HOST GAME");
   menuItems.push_back("OPTIONS");
   menuItems.push_back("QUIT");
}

void MainMenuUserInterface::processSelection(U32 index)
{
   switch(index)
   {
      case 0:
         gQueryServersUserInterface.activate();
         break;
      case 1:
         gIPEntryUserInterface.activate();
         break;
      case 2:
         hostGame(false, Address(IPProtocol, Address::Any, 28000));
         break;
      case 3:
         gOptionsMenuUserInterface.activate();
         break;
      case 4:
         gCreditsUserInterface.activate();
         break;
   }
}

void MainMenuUserInterface::onEscape()
{
   gNameEntryUserInterface.activate();
}


OptionsMenuUserInterface gOptionsMenuUserInterface;

bool OptionsMenuUserInterface::controlsRelative = false;
bool OptionsMenuUserInterface::fullscreen = false;
bool OptionsMenuUserInterface::joystickEnabled = false;
bool OptionsMenuUserInterface::echoVoice = false;

OptionsMenuUserInterface::OptionsMenuUserInterface()
{
   menuTitle = "OPTIONS MENU:";
}

void OptionsMenuUserInterface::onActivate()
{
   setupMenus();
}

void OptionsMenuUserInterface::setupMenus()
{
   clearBackground = !gClientGame->getConnectionToServer();

   menuItems.clear();
   if(controlsRelative)
      menuItems.push_back("SET CONTROLS TO ABSOLUTE");
   else
      menuItems.push_back("SET CONTROLS TO RELATIVE");

   if(fullscreen)
      menuItems.push_back("SET WINDOWED MODE");
   else
      menuItems.push_back("SET FULLSCREEN MODE");

   if(joystickEnabled)
      menuItems.push_back("DISABLE JOYSTICK/CONTROLLER");
   else
      menuItems.push_back("ENABLE JOYSTICK/CONTROLLER");

   if(echoVoice)
      menuItems.push_back("DISABLE VOICE ECHO");
   else
      menuItems.push_back("ENABLE VOICE ECHO");

   if(gClientGame->getConnectionToServer())
      menuItems.push_back("RETURN TO GAME");
   else
      menuItems.push_back("GO TO MAIN MENU");
}

void OptionsMenuUserInterface::render()
{
   if(!clearBackground)
   {
      gGameUserInterface.render();
      glColor4f(0, 0, 0, 0.5);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_POLYGON);
      glVertex2f(0, 0);
      glVertex2f(canvasWidth, 0);
      glVertex2f(canvasWidth, canvasHeight);
      glVertex2f(0, canvasHeight);
      glEnd();  
      glDisable(GL_BLEND); 
      glBlendFunc(GL_ONE, GL_ZERO);
   }
   Parent::render();
}

void OptionsMenuUserInterface::processSelection(U32 index)
{
   switch(index)
   {
   case 0:
      controlsRelative = !controlsRelative;
      break;
   case 1:
      if(fullscreen)
      {
         glutPositionWindow(100, 100);
         glutReshapeWindow(800, 600);
      }
      else
         glutFullScreen();
      fullscreen = !fullscreen;
      break;
   case 2:
      joystickEnabled = !joystickEnabled;
      break;
   case 3:
      echoVoice = !echoVoice;
      break;
   case 4:
      if(gClientGame->getConnectionToServer())
         gGameUserInterface.activate();   
      else
         gMainMenuUserInterface.activate();
      break;
   };
   setupMenus();
}

void OptionsMenuUserInterface::onEscape()
{
   if(gClientGame->getConnectionToServer())
      gGameUserInterface.activate();   
   else
      gMainMenuUserInterface.activate();
}


GameMenuUserInterface gGameMenuUserInterface;

GameMenuUserInterface::GameMenuUserInterface()
{
   menuTitle = "GAME MENU:";
   clearBackground = false;
}

void GameMenuUserInterface::onActivate()
{
   menuItems.clear();
   menuItems.push_back("RETURN TO GAME");
   menuItems.push_back("OPTIONS");
   menuItems.push_back("LEAVE GAME");
   GameType *theGameType = gClientGame->getGameType();
   if(theGameType)
   {
      mGameType = theGameType;
      theGameType->addClientGameMenuOptions(menuItems);
   }
}

void GameMenuUserInterface::processSelection(U32 index)
{
   switch(index)
   {
      case 0:
         gGameUserInterface.activate();
         break;
      case 1:
         gOptionsMenuUserInterface.activate();
         break;
      case 2:
         endGame();
         if(EditorUserInterface::editorEnabled)
            gEditorUserInterface.activate();
         else
            gMainMenuUserInterface.activate();
         break;
      default:
         if(mGameType.isValid())
            mGameType->processClientGameMenuOption(index - 3);
         gGameUserInterface.activate();
         break;
   }
}

void GameMenuUserInterface::onEscape()
{
   gGameUserInterface.activate();
}

void GameMenuUserInterface::render()
{
   gGameUserInterface.render();
   glColor4f(0, 0, 0, 0.5);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBegin(GL_POLYGON);
   glVertex2f(0, 0);
   glVertex2f(canvasWidth, 0);
   glVertex2f(canvasWidth, canvasHeight);
   glVertex2f(0, canvasHeight);
   glEnd();  
   glDisable(GL_BLEND); 
   glBlendFunc(GL_ONE, GL_ZERO);
   Parent::render();
}

};

