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
#include "../tnl/tnlRandom.h"
#include "../tnl/tnlGhostConnection.h"
#include "../tnl/tnlNetInterface.h"

#include "glutInclude.h"
#include <stdarg.h>

using namespace TNL;
#include "UI.h"
#include "UIGame.h"
#include "UINameEntry.h"
#include "UIMenus.h"
#include "game.h"
#include "gameNetInterface.h"
#include "masterConnection.h"
#include "sfx.h"

namespace Zap
{

bool gQuit = false;
bool gIsServer = false;
const char *gHostName = "ZAP Game";
const char *gWindowTitle = "ZAP II - The Return";
U32 gMaxPlayers = 128;

const char *gMasterAddressString = "IP:master.opentnl.org:29005";
Address gMasterAddress;
const char *gLevelList = "level1.txt level3.txt";

void reshape(int nw, int nh)
{
  UserInterface::windowWidth = nw;
  UserInterface::windowHeight = nh;
}

void motion(int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onMouseDragged(x, y);
}

void passivemotion(int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onMouseMoved(x, y);
}

void key(unsigned char key, int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onKeyDown(key);
}

void keyup(unsigned char key, int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onKeyUp(key);
}

void mouse(int button, int state, int x, int y)
{
   static int mouseState[2] = { 0, };
   if(!UserInterface::current)
      return;

   if(button == GLUT_LEFT_BUTTON)
   {
      if(state == 1 && !mouseState[0])
      {
         UserInterface::current->onMouseUp(x, y);
         mouseState[0] = 0;
      }
      else
      {
         mouseState[0] = state;
         UserInterface::current->onMouseDown(x, y);
      }
   }
   else if(button == GLUT_RIGHT_BUTTON)
   {
      if(state == 1 && !mouseState[1])
      {
         UserInterface::current->onRightMouseUp(x, y);
         mouseState[1] = 0;
      }
      else
      {
         mouseState[1] = state;
         UserInterface::current->onRightMouseDown(x, y);
      }
   }
}

void specialkey(int key, int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onSpecialKeyDown(key);
}

void specialkeyup(int key, int x, int y)
{
   if(UserInterface::current)
      UserInterface::current->onSpecialKeyUp(key);
}

void idle()
{
   static S64 lastTimer = Platform::getHighPrecisionTimerValue();
   static F64 unusedFraction = 0;

   S64 currentTimer = Platform::getHighPrecisionTimerValue();

   F64 timeElapsed = Platform::getHighPrecisionMilliseconds(currentTimer - lastTimer) + unusedFraction;
   U32 integerTime = timeElapsed;

   if(integerTime >= 10)
   {
      lastTimer = currentTimer;
      unusedFraction = timeElapsed - integerTime;

      if(UserInterface::current)
         UserInterface::current->idle(integerTime);
      if(gClientGame)
         gClientGame->idle(integerTime);
      if(gServerGame)
         gServerGame->idle(integerTime);
      if(gClientGame)
         glutPostRedisplay();
   }

   // Sleep a bit so we don't saturate the system. For a non-dedicated server,
   // sleep(0) helps reduce the impact of OpenGL on windows.
   Platform::sleep((gClientGame ? 0 : 1));
}

void dedicatedServerLoop()
{
   for(;;)
      idle();
}

void display(void)
{
   if(UserInterface::current)
      UserInterface::current->render();

   // Render master connection state...
   if(gClientGame && gClientGame->getConnectionToMaster() 
      && gClientGame->getConnectionToMaster()->getConnectionState() != NetConnection::Connected)
   {
      glColor3f(1,1,1);
      UserInterface::drawStringf(10, 550, 15, "Master Server - %s", 
                                 gConnectStatesTable[gClientGame->getConnectionToMaster()->getConnectionState()]);

   }
   glFlush();
   glutSwapBuffers();

}

#include <stdio.h>
class StdoutLogConsumer : public LogConsumer
{
public:
   void logString(const char *string)
   {
      printf("%s\r\n", string);
   }
} gStdoutLogConsumer;

class FileLogConsumer : public LogConsumer
{
private:
   FILE *f;
public:
   FileLogConsumer(const char* logFile="zap.log")
   {
      f = fopen(logFile, "w");
      logString("------ Zap Log File ------");
   }

   ~FileLogConsumer()
   {
      if(f)
         fclose(f);
   }

   void logString(const char *string)
   {
      if(f)
         fprintf(f, "%s\r\n", string);
   }
} gFileLogConsumer;

void hostGame(bool dedicated, Address bindAddress)
{
   gServerGame = new ServerGame(bindAddress, gMaxPlayers, gHostName);
   gServerGame->setLevelList(gLevelList);

   if(!dedicated)
      joinGame(Address(), false, true);

}


void joinGame(Address remoteAddress, bool isFromMaster, bool local)
{
   if(isFromMaster && gClientGame->getConnectionToMaster())
   {
      gClientGame->getConnectionToMaster()->requestArrangedConnection(remoteAddress);
      gGameUserInterface.activate();
   }
   else
   {
      GameConnection *theConnection = new GameConnection(gClientGame);
      gClientGame->setConnectionToServer(theConnection);

      const char *name = gNameEntryUserInterface.getText();
      if(!name[0])
         name = "Playa";

      theConnection->setPlayerName(name);

      if(local)
         theConnection->connectLocal(gClientGame->getNetInterface(), gServerGame->getNetInterface());
      else
         theConnection->connect(gClientGame->getNetInterface(), remoteAddress);
      gGameUserInterface.activate();
   }
}

void endGame()
{
   if(gClientGame && gClientGame->getConnectionToMaster())
      gClientGame->getConnectionToMaster()->cancelArrangedConnectionAttempt();

   if(gClientGame && gClientGame->getConnectionToServer())
      gClientGame->getConnectionToServer()->disconnect("");
   delete gServerGame;
   gServerGame = NULL;
}

void onExit()
{
   endGame();
   SFXObject::shutdown();
}

extern void InitController();

};

using namespace Zap;

int main(int argc, char **argv)
{
   TNLLogEnable(LogNetInterface, true);
   TNLLogEnable(LogPlatform, true);
   bool hasClient = true;
   bool hasServer = false;
   bool connectLocal = false;
   bool connectRemote = false;
   bool nameSet = false;

   Address connectAddress;
   Address bindAddress(IPProtocol, Address::Any, 28000);
   U32 maxPlayers = 128;

   for(S32 i = 1; i < argc;i+=2)
   {
      bool hasAdditionalArg = (i != argc - 1);

      if(!stricmp(argv[i], "-server"))
      {
         hasServer = true;
         connectLocal = true;
         if(hasAdditionalArg)
            bindAddress.set(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-connect"))
      {
         connectRemote = true;
         if(hasAdditionalArg)
            connectAddress.set(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-master"))
      {
         if(hasAdditionalArg)
            gMasterAddressString = argv[i+1];
      }
      else if(!stricmp(argv[i], "-dedicated"))
      {
         hasClient = false;
         hasServer = true;
         if(hasAdditionalArg)
            bindAddress.set(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-name"))
      {
         if(hasAdditionalArg)
         {
            nameSet = true;
            gNameEntryUserInterface.setText(argv[i+1]);
         }
      }
      else if(!stricmp(argv[i], "-levels"))
      {
         if(hasAdditionalArg)
            gLevelList = argv[i+1];
      }
      else if(!stricmp(argv[i], "-hostname"))
      {
         if(hasAdditionalArg)
            gHostName = argv[i+1];
      }
      else if(!stricmp(argv[i], "-maxplayers"))
      {
         if(hasAdditionalArg)
            gMaxPlayers = atoi(argv[i+1]);
      }
   }
   gMasterAddress.set(gMasterAddressString);

   if(hasClient)
      gClientGame = new ClientGame(Address());

   if(hasServer)
      hostGame(hasClient == false, bindAddress);
   else if(connectRemote)
      joinGame(connectAddress, false);

   if(!connectLocal && !connectRemote)
   {
      if(!nameSet)
         gNameEntryUserInterface.activate();
      else
         gMainMenuUserInterface.activate();
   }
   if(hasClient)
   {
      SFXObject::init();
      glutInitWindowSize(800, 600);
      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
      glutCreateWindow(gWindowTitle);
      glutDisplayFunc(display);
      glutReshapeFunc(reshape);
      glutPassiveMotionFunc(passivemotion);
      glutMotionFunc(passivemotion);
      glutKeyboardFunc(key);
      glutKeyboardUpFunc(keyup);
      glutSpecialFunc(specialkey);
      glutSpecialUpFunc(specialkeyup);
      glutMouseFunc(mouse);
      glutIdleFunc(idle);

#ifdef TNL_OS_WIN32
      InitController();
#endif

      glutSetCursor(GLUT_CURSOR_NONE);
      glMatrixMode(GL_PROJECTION);
      glOrtho(0, 800, 600, 0, 0, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(400, 300, 0);

      atexit(endGame);
      glutMainLoop();
   }
   else
   {
      dedicatedServerLoop();
   }
   return 0;
}

