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

#ifndef _UIMENUS_H_
#define _UIMENUS_H_

#include "tnlNetBase.h"
#include "UI.h"

namespace Zap
{

class MenuUserInterface : public UserInterface
{
public:
   Vector<const char *> menuItems;
   const char *menuTitle;
   const char *menuSubTitle;
   const char *menuFooter;

   bool clearBackground;

   S32 selectionIndex;

   MenuUserInterface() 
   {
      clearBackground = true;
      menuTitle = "Menu";
      menuSubTitle = "";
      menuFooter = "";
      selectionIndex = 0;
   }

   void render();
   void onSpecialKeyDown(U32 key);
   void onKeyDown(U32 key);
   void onActivate() { selectionIndex = 0; }

   virtual void onEscape();
   virtual void processSelection(U32 index) = 0;
};

class MainMenuUserInterface : public MenuUserInterface
{
   typedef MenuUserInterface Parent;
   char titleBuffer[256];
   char motd[256];
   U32 motdArriveTime;
public:
   MainMenuUserInterface();
   void processSelection(U32 index);
   void onEscape();
   void render();
   void setMOTD(const char *motdString);
};

extern MainMenuUserInterface gMainMenuUserInterface;

class OptionsMenuUserInterface : public MenuUserInterface
{
   typedef MenuUserInterface Parent;
public:
   static bool controlsRelative;
   static bool fullscreen;
   static S32 joystickType;
   static bool echoVoice;

   OptionsMenuUserInterface();
   void processSelection(U32 index);
   void onEscape();
   void setupMenus();
   void onActivate();
   void render();
};

extern OptionsMenuUserInterface gOptionsMenuUserInterface;

class GameType;

class GameMenuUserInterface : public MenuUserInterface
{
   typedef MenuUserInterface Parent;
   SafePtr<GameType> mGameType;
public:
   GameMenuUserInterface();
   void render();
   void onActivate();
   void processSelection(U32 index);
   void onEscape();
};

extern GameMenuUserInterface gGameMenuUserInterface;

};

#endif
