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

#ifndef _UIGAME_H_
#define _UIGAME_H_

#include "UI.h"
#include "point.h"
#include "gameConnection.h"

namespace Zap
{

class GameUserInterface : public UserInterface
{
   Move mCurrentMove;
   Move mTransformedMove;

   Point mMousePoint;
   enum {
      DisplayMessageTimeout = 2500,
   };

   enum {
      MessageDisplayCount = 4,
   };

   Color mDisplayMessageColor[MessageDisplayCount];
   char mDisplayMessage[MessageDisplayCount][2048];


   U32 mDisplayMessageTimer;
   enum Mode {
      PlayMode,
      ChatMode,
   };
   enum ChatType {
      GlobalChat,
      TeamChat,
   };
   enum {
      ChatBlinkTime = 100,
   };
   Mode mCurrentMode;
   ChatType mCurrentChatType;

   char mChatBuffer[50];
   U32 mChatCursorPos;
   bool mChatBlink;
   U32 mChatLastBlinkTime;

public:
   GameUserInterface();

   void displayMessage(Color messageColor, const char *format, ...);

   void render();
   void idle(U32 timeDelta);

   void issueChat();
   void cancelChat();

   void onMouseMoved(S32 x, S32 y);
   void onMouseDragged(S32 x, S32 y);
   void onMouseDown(S32 x, S32 y);
   void onMouseUp(S32 x, S32 y);
   void onKeyDown(U32 key);
   void onKeyUp(U32 key);

   Move *getCurrentMove();
};

extern GameUserInterface gGameUserInterface;

};

#endif
