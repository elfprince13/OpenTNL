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

#include "gameConnection.h"

#include "game.h"
#include "UIGame.h"
#include "UIMenus.h"
#include "gameType.h"
#include "lpc10.h"
#include "tnlEndian.h"

#include <stdarg.h>
#include "glutInclude.h"
#include <ctype.h>
#include <stdio.h>

namespace Zap
{

GameUserInterface gGameUserInterface;
Color gGlobalChatColor(0.9, 0.9, 0.9);
Color gTeamChatColor(0, 1, 0);

GameUserInterface::GameUserInterface()
{
   mCurrentMode = PlayMode;
   mInScoreboardMode = false;

   mChatLastBlinkTime = 0;
   memset(mChatBuffer, 0, sizeof(mChatBuffer));
   mChatBlink = false;

   for(U32 i = 0; i < MessageDisplayCount; i++)
      mDisplayMessage[i][0] = 0;

   mVChat = new VChatHelper();
   mRecordingAudio = false;
   mMaxAudioSample = 0;
   mVoiceEncoder = new LPC10VoiceEncoder;
}

void GameUserInterface::displayMessage(Color theColor, const char *format, ...)
{
   for(S32 i = MessageDisplayCount - 1; i > 0; i--)
   {
      strcpy(mDisplayMessage[i], mDisplayMessage[i-1]);
      mDisplayMessageColor[i] = mDisplayMessageColor[i-1];
   }
   va_list args;
   va_start(args, format);
   dVsprintf(mDisplayMessage[0], sizeof(mDisplayMessage[0]), format, args);
   mDisplayMessageColor[0] = theColor;

   mDisplayMessageTimer = DisplayMessageTimeout;
}

void GameUserInterface::idle(U32 timeDelta)
{
   if(timeDelta > mDisplayMessageTimer)
   {
      mDisplayMessageTimer = DisplayMessageTimeout;
      for(S32 i = MessageDisplayCount - 1; i > 0; i--)
      {
         strcpy(mDisplayMessage[i], mDisplayMessage[i-1]);
         mDisplayMessageColor[i] = mDisplayMessageColor[i-1];
      }

      mDisplayMessage[0][0] = 0;
   }
   else
      mDisplayMessageTimer -= timeDelta;
   if(mCurrentMode == ChatMode)
   {
      mChatLastBlinkTime += timeDelta;
      if(mChatLastBlinkTime > ChatBlinkTime)
      {
         mChatBlink = !mChatBlink;
         mChatLastBlinkTime = 0;
      }
   }
   if(mRecordingAudio)
   {
      if(mVoiceAudioTimer.update(timeDelta))
      {
         mVoiceAudioTimer.reset(VoiceAudioSampleTime);
         processRecordingAudio();
      }
   }
}

#ifdef TNL_OS_WIN32
extern void checkMousePos(S32 maxdx, S32 maxdy);
#endif

void GameUserInterface::render()
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glColor3f(0.0, 0.0, 0.0);
   glViewport(0, 0, windowWidth, windowHeight);

   glClearColor(0, 0, 0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if(!gClientGame->isConnectedToServer())
   {
      glColor3f(1,1,1);
      drawCenteredString(290, 30, "Connecting to server...");

      if(gClientGame->getConnectionToServer())
         drawCenteredString(330, 16, gConnectStatesTable[gClientGame->getConnectionToServer()->getConnectionState()]);
      drawCenteredString(370, 20, "Press <ESC> to abort");
   }

   if(gClientGame)
      gClientGame->render();

   // draw the reticle
   if(!OptionsMenuUserInterface::joystickEnabled)
   {
      Point realMousePoint = mMousePoint;

#ifdef TNL_OS_WIN32

      F32 len = mMousePoint.len();
      checkMousePos(windowWidth * 100 / canvasWidth,
                    windowHeight * 100 / canvasHeight);

      if(len > 100)
         realMousePoint *= 100 / len;
#endif

      glPushMatrix();
      glTranslatef(400, 300, 0);

      glTranslatef(realMousePoint.x, realMousePoint.y, 0);

      static U32 cursorSpin = 90;
      cursorSpin++;

      glRotatef((cursorSpin % 720) * 0.9, 0, 0, 1);

      glColor3f(1, 0, 0);
      glBegin(GL_LINE_LOOP);
      glVertex2f(-8, -6);
      glVertex2f(-6, -8);
      glVertex2f(-3, -3);
      glEnd();
      glBegin(GL_LINE_LOOP);
      glVertex2f( 8, 6);
      glVertex2f( 6, 8);
      glVertex2f( 3, 3);
      glEnd();
      glBegin(GL_LINE_LOOP);
      //glColor3f(1, 0, 0);
      glVertex2f(8, -6);
      glVertex2f(6, -8);
      glVertex2f(3, -3);
      glEnd();
      glBegin(GL_LINE_LOOP);
      glVertex2f(-8, 6);
      glVertex2f(-6, 8);
      glVertex2f(-3, 3);

      glEnd();
      glPopMatrix();
   }
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glColor3f(1,1,1);

   U32 y = 5;
   for(S32 i = 3; i >= 0; i--)
   {
      if(mDisplayMessage[i][0])
      {
         glColor3f(mDisplayMessageColor[i].r, 
                  mDisplayMessageColor[i].g,
                  mDisplayMessageColor[i].b);

         drawString(5, y, 20, mDisplayMessage[i]);
         y += 24;
      }
   }
   GameType *theGameType = gClientGame->getGameType();

   if(theGameType)
      theGameType->renderInterfaceOverlay(mInScoreboardMode);

   if(mCurrentMode == ChatMode)
   {
      const char *promptStr;
      if(mCurrentChatType == TeamChat)
      {
         glColor3f(gTeamChatColor.r,
                   gTeamChatColor.g,
                   gTeamChatColor.b);

         promptStr = "(Team): ";
      }
      else
      {
         glColor3f(gGlobalChatColor.r,
                   gGlobalChatColor.g,
                   gGlobalChatColor.b);

         promptStr = "(Global): ";
      }

      U32 width = getStringWidth(20, promptStr);

      drawString(5, 100, 20, promptStr);
      drawString(5 + width, 100, 20, mChatBuffer);

      if(mChatBlink)
         drawString(5 + width + getStringWidth(20, mChatBuffer, mChatCursorPos), 100, 20, "_");
   }

   if(mRecordingAudio)
   {
      F32 amt = mMaxAudioSample / F32(0x7FFF);
      U32 totalLineCount = 50;

      glColor3f(1, 1 ,1);
      glBegin(GL_LINES);
      glVertex2f(10, 130);
      glVertex2f(10, 145);
      glVertex2f(10 + totalLineCount * 2, 130);
      glVertex2f(10 + totalLineCount * 2, 145);
      glColor3f(0.8, 0.8, 0.8);

      for(U32 i = 1; i < amt * totalLineCount; i++)
      {
         glVertex2f(10 + i * 2, 130);
         glVertex2f(10 + i * 2, 145);
      }
      glEnd();
   }

#if 0
   // some code for outputting the position of the ship for finding good spawns
   GameConnection *con = gClientGame->getConnectionToServer();

   if(con)
   {
      GameObject *co = con->getControlObject();
      if(co)
      {
         Point pos = co->getActualPos() * F32(1 / 300.0f);
         drawStringf(10, 550, 30, "%0.2g, %0.2g", pos.x, pos.y);
      }
   }
#endif

   if(mVChat->isActive())
      mVChat->render();
}

void GameUserInterface::onMouseDragged(S32 x, S32 y)
{
   onMouseMoved(x, y);
}

void GameUserInterface::onMouseMoved(S32 x, S32 y)
{
   S32 xp = x - windowWidth / 2;
   S32 yp = y - windowHeight / 2;
   S32 horzMax = 100 * windowWidth / canvasWidth;
   S32 vertMax = 100 * windowHeight / canvasHeight;

   
   mMousePoint = Point(x - windowWidth / 2, y - windowHeight / 2);
   mMousePoint.x *= canvasWidth / windowWidth;
   mMousePoint.y *= canvasHeight / windowHeight;
   mCurrentMove.angle = atan2(mMousePoint.x, mMousePoint.y);
}

void GameUserInterface::onMouseDown(S32 x, S32 y)
{
   mCurrentMove.fire = true;
}

void GameUserInterface::onMouseUp(S32 x, S32 y)
{
   mCurrentMove.fire = false;
}

void GameUserInterface::onRightMouseDown(S32 x, S32 y)
{
   mCurrentMove.shield = true;
}

void GameUserInterface::onRightMouseUp(S32 x, S32 y)
{
   mCurrentMove.shield = false;
}

void GameUserInterface::onControllerButtonDown(U32 buttonIndex)
{
   switch(buttonIndex)
   {
      case 0:
         startRecordingAudio();
         break;

      case 3:
      {
         mInScoreboardMode = true;
         GameType *g = gClientGame->getGameType();
         if(g)
            g->c2sRequestScoreboardUpdates(true);
         break;
      }
      case 2:
         gClientGame->zoomCommanderMap();
         break;

      case 7:
         mCurrentMove.boost = true;
         break;

      case 6:
         mCurrentMove.shield = true;
         break;
   }
}

void GameUserInterface::onControllerButtonUp(U32 buttonIndex)
{
   switch(buttonIndex)
   {
      case 0:
         stopRecordingAudio();
         break;
      case 3:
      {
         mInScoreboardMode = false;
         GameType *g = gClientGame->getGameType();
         if(g)
            g->c2sRequestScoreboardUpdates(false);
         break;
      }

      case 7:
         mCurrentMove.boost = false;
         break;

      case 6:
         mCurrentMove.shield = false;
         break;
   }
}

void GameUserInterface::onKeyDown(U32 key)
{
   if(mCurrentMode == PlayMode)
   {
      mCurrentChatType = GlobalChat;

      switch(toupper(key))
      {
         case '\t':
         {
            mInScoreboardMode = true;
            GameType *g = gClientGame->getGameType();
            if(g)
               g->c2sRequestScoreboardUpdates(true);
            break;
         }
         case 'W':
            mCurrentMove.up = 1.0;
            break;
         case 'A':
            mCurrentMove.left = 1.0;
            break;
         case 'S':
            mCurrentMove.down = 1.0;
            break;
         case 'D':
            mCurrentMove.right = 1.0;
            break;
         case ' ':
            mCurrentMove.boost = true;
            break;

         case 27:
            if(!gClientGame->isConnectedToServer())
            {
               endGame();
               gMainMenuUserInterface.activate();
            }
            else
               gGameMenuUserInterface.activate();
            break;

         case 'T':
            mCurrentChatType = TeamChat;
         case 'G':
            mChatLastBlinkTime = 0;
            mChatBlink = true;
            mCurrentMode = ChatMode;
            mCurrentMove.up = 
               mCurrentMove.left = 
               mCurrentMove.right = 
               mCurrentMove.down = 0;
            break;
         case 'V':
            UserInterface::playBoop();
            mVChat->show();
            mCurrentMode = VChatMode;
            break;
         case 'C':
            gClientGame->zoomCommanderMap();
            break;
         case 'R':
            startRecordingAudio();
            break;
      }
   }
   else if(mCurrentMode == ChatMode)
   {
      if(key == '\r')
         issueChat();
      else if(key == 8)
      {
         // backspace key
         if(mChatCursorPos > 0)
         {
            mChatCursorPos--;
            for(U32 i = mChatCursorPos; mChatBuffer[i]; i++)
               mChatBuffer[i] = mChatBuffer[i+1];
         }
      }
      else if(key == 27)
      {
         cancelChat();
      }
      else
      {
         for(U32 i = sizeof(mChatBuffer) - 2; i > mChatCursorPos; i--)
            mChatBuffer[i] = mChatBuffer[i-1];
         if(mChatCursorPos < sizeof(mChatBuffer) - 2)
         {
            mChatBuffer[mChatCursorPos] = key;
            mChatCursorPos++;
         }
      }
   }
   else if(mCurrentMode == VChatMode)
   {
      mVChat->processKey(key);

      if(!mVChat->isActive())
         mCurrentMode = PlayMode;
   }
}

void GameUserInterface::onKeyUp(U32 key)
{
   if(mCurrentMode == PlayMode)
   {
      switch(toupper(key))
      {
         case '\t':
         {
            mInScoreboardMode = false;
            GameType *g = gClientGame->getGameType();
            if(g)
               g->c2sRequestScoreboardUpdates(false);
            break;
         }
         case 'W':
            mCurrentMove.up = 0;
            break;
         case 'A':
            mCurrentMove.left = 0;
            break;
         case 'S':
            mCurrentMove.down = 0;
            break;
         case 'D':
            mCurrentMove.right = 0;
            break;

         case ' ':
            mCurrentMove.boost = false;
            break;
         case 'F':
            mCurrentMove.shield = !mCurrentMove.shield;
            break;
         case 'R':
            stopRecordingAudio();
            break;
      }
   }
   else if(mCurrentMode == ChatMode)
   {


   }
}

void GameUserInterface::startRecordingAudio()
{
   if(!mRecordingAudio)
   {
      mUnusedAudio = new ByteBuffer(0);
      mRecordingAudio = true;
      mMaxAudioSample = 0;
      mVoiceAudioTimer.reset(FirstVoiceAudioSampleTime);
      SFXObject::startRecording();

      // trim the start of the capture buffer:
      SFXObject::captureSamples(mUnusedAudio);
      mUnusedAudio->resize(0);
   }
}

void GameUserInterface::stopRecordingAudio()
{
   if(mRecordingAudio)
   {
      processRecordingAudio();
      SFXObject::stopRecording();

      mRecordingAudio = false;
      SFXObject::stopRecording();
      mVoiceSfx = NULL;
      mUnusedAudio = NULL;
   }
}

void GameUserInterface::processRecordingAudio()
{
   SFXObject::captureSamples(mUnusedAudio);
   ByteBufferPtr sendBuffer = mVoiceEncoder->compressBuffer(mUnusedAudio);

   U32 sampleCount = mUnusedAudio->getBufferSize() / 2;
   S16 *samplePtr = (S16 *) mUnusedAudio->getBuffer();
   mMaxAudioSample = 0;

   for(U32 i = 0; i < sampleCount; i++)
      if(samplePtr[i] > mMaxAudioSample)
         mMaxAudioSample = samplePtr[i];

   if(sendBuffer.isValid())
   {
      GameType *gt = gClientGame->getGameType();
      if(gt)
         gt->c2sVoiceChat(OptionsMenuUserInterface::echoVoice, *(sendBuffer.getPointer()));
   }
}

Move *GameUserInterface::getCurrentMove()
{
   if(!OptionsMenuUserInterface::controlsRelative)
      return &mCurrentMove;
   else
   {
      mTransformedMove = mCurrentMove;

      Point moveDir(mCurrentMove.right - mCurrentMove.left,
                    mCurrentMove.up - mCurrentMove.down);

      Point angleDir(sin(mCurrentMove.angle), cos(mCurrentMove.angle));

      Point rightAngleDir(-angleDir.y, angleDir.x);
      Point newMoveDir = angleDir * moveDir.y + rightAngleDir * moveDir.x;

      if(newMoveDir.x > 0)
      {
         mTransformedMove.right = newMoveDir.x;
         mTransformedMove.left = 0;
      }
      else
      {
         mTransformedMove.right = 0;
         mTransformedMove.left = -newMoveDir.x;
      }
      if(newMoveDir.y > 0)
      {
         mTransformedMove.down = newMoveDir.y;
         mTransformedMove.up = 0;
      }
      else
      {
         mTransformedMove.down = 0;
         mTransformedMove.up = -newMoveDir.y;
      }
      if(mTransformedMove.right > 1)
         mTransformedMove.right = 1;
      if(mTransformedMove.left > 1)
         mTransformedMove.left = 1;
      if(mTransformedMove.up > 1)
         mTransformedMove.up = 1;
      if(mTransformedMove.down > 1)
         mTransformedMove.down = 1;

      return &mTransformedMove;
   }
}

void GameUserInterface::cancelChat()
{
   memset(mChatBuffer, 0, sizeof(mChatBuffer));
   mChatCursorPos = 0;
   mCurrentMode = PlayMode;
}

void GameUserInterface::issueChat()
{
   if(mChatBuffer[0])
   {
      GameType *gt = gClientGame->getGameType();
      if(gt)
         gt->c2sSendChat(mCurrentChatType == GlobalChat, mChatBuffer);
   }
   cancelChat();
}

};
