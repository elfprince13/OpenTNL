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
#include "../tnl/tnlJournal.h"

#include "glutInclude.h"
#include <stdarg.h>
#include <direct.h>

using namespace TNL;
#include "UI.h"
#include "UIGame.h"
#include "UINameEntry.h" 
#include "UIMenus.h"
#include "game.h"
#include "gameNetInterface.h"
#include "masterConnection.h"
#include "sfx.h"
#include "sparkManager.h"

namespace Zap
{

bool gIsCrazyBot = false;
bool gQuit = false;
bool gIsServer = false;
const char *gHostName = "ZAP Game";
const char *gWindowTitle = "ZAP II - The Return";
U32 gMaxPlayers = 128;
U32 gJoystickType = 0;
U32 gSimulatedPing = 0;
F32 gSimulatedPacketLoss = 0;

const char *gMasterAddressString = "IP:master.opentnl.org:29005";
Address gMasterAddress;
Address gConnectAddress;
Address gBindAddress(IPProtocol, Address::Any, 28000);

const char *gLevelList = "level4.txt level2.txt level1.txt level2.txt level3.txt level2.txt";

class ZapJournal : public Journal
{
public:
   TNL_DECLARE_JOURNAL_ENTRYPOINT(reshape, (S32 newWidth, S32 newHeight));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(motion, (S32 x, S32 y));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(passivemotion, (S32 x, S32 y));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(key, (U8 key));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(keyup, (U8 key));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(mouse, (S32 button, S32 state, S32 x, S32 y));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(specialkey, (S32 key));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(specialkeyup, (S32 key));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(idle, (U32 timeDelta));
   TNL_DECLARE_JOURNAL_ENTRYPOINT(display, ());
   TNL_DECLARE_JOURNAL_ENTRYPOINT(startup, (bool hasClient, bool hasServer, bool connectLocal, bool connectRemote, bool nameSet));
};

ZapJournal gZapJournal;

void reshape(int nw, int nh)
{
   gZapJournal.reshape(nw, nh);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, reshape, (S32 newWidth, S32 newHeight))
{
  UserInterface::windowWidth = newWidth;
  UserInterface::windowHeight = newHeight;
}

void motion(int x, int y)
{
   gZapJournal.motion(x, y);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, motion, (S32 x, S32 y))
{
   if(gIsCrazyBot)
      return;

   if(UserInterface::current)
      UserInterface::current->onMouseDragged(x, y);
}

void passivemotion(int x, int y)
{
   gZapJournal.passivemotion(x, y);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, passivemotion, (S32 x, S32 y))
{
   if(gIsCrazyBot)
      return;

   if(UserInterface::current)
      UserInterface::current->onMouseMoved(x, y);
}

void key(unsigned char key, int x, int y)
{
   gZapJournal.key(key);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, key, (U8 key))
{
   if(UserInterface::current)
      UserInterface::current->onKeyDown(key);
}

void keyup(unsigned char key, int x, int y)
{
   gZapJournal.keyup(key);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, keyup, (U8 key))
{
   if(UserInterface::current)
      UserInterface::current->onKeyUp(key);
}

void mouse(int button, int state, int x, int y)
{
   gZapJournal.mouse(button, state, x, y);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, mouse, (S32 button, S32 state, S32 x, S32 y))
{
   static int mouseState[2] = { 0, };
   if(!UserInterface::current)
      return;

   if(gIsCrazyBot)
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
   gZapJournal.specialkey(key);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, specialkey, (S32 key))
{
   if(UserInterface::current)
      UserInterface::current->onSpecialKeyDown(key);
}

void specialkeyup(int key, int x, int y)
{
   gZapJournal.specialkeyup(key);
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, specialkeyup, (S32 key))
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

      gZapJournal.idle(integerTime);
   }

   // Make us move all crazy like...
   if(gIsCrazyBot)
   {
      gIsCrazyBot = false; // Reenable input events
      static S64 lastMove = Platform::getHighPrecisionTimerValue();

      F64 delta = Platform::getHighPrecisionMilliseconds(currentTimer - lastMove);
      if(delta > 200.0)
      {
            // Do movement craziness
            if(Random::readB())
               gZapJournal.key('w');
            else
               gZapJournal.keyup('w');

            if(Random::readB())
               gZapJournal.key('a');
            else
               gZapJournal.keyup('a');

            if(Random::readB())
               gZapJournal.key('s');
            else
               gZapJournal.keyup('s');

            if(Random::readB())
               gZapJournal.key('d');
            else
               gZapJournal.keyup('d');

            if(Random::readB())
               gZapJournal.key('r');
            else
               gZapJournal.keyup('r');

            if(Random::readB())
               gZapJournal.key('c');
            else
               gZapJournal.keyup('c');

            if(Random::readB())
               gZapJournal.key('\t');
            else
               gZapJournal.keyup('\t');

            if(Random::readB())
               gZapJournal.key(' ');
            else
               gZapJournal.keyup(' ');

            // Do mouse craziness
            S32 x = Random::readI(0, 800);
            S32 y = Random::readI(0, 600);
            gZapJournal.passivemotion(x,y);
            gZapJournal.mouse(0, Random::readF() > 0.2, x,y);
            gZapJournal.mouse(1, Random::readF() > 0.2, x,y);
            gZapJournal.mouse(2, Random::readF() > 0.8, x,y);
            lastMove = currentTimer;
      }
      gIsCrazyBot = true; // Reenable input events
   }



   // Sleep a bit so we don't saturate the system. For a non-dedicated server,
   // sleep(0) helps reduce the impact of OpenGL on windows.
   U32 sleepTime = 1;

   if(gClientGame) sleepTime = 0;
   if(gIsCrazyBot) sleepTime = 10;

   Platform::sleep(sleepTime);
   gZapJournal.processNextJournalEntry();
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, idle, (U32 integerTime))
{
   if(UserInterface::current)
      UserInterface::current->idle(integerTime);
   if(gClientGame)
      gClientGame->idle(integerTime);
   if(gServerGame)
      gServerGame->idle(integerTime);
   if(gClientGame)
      glutPostRedisplay();
}

void dedicatedServerLoop()
{
   for(;;)
      idle();
}

void display(void)
{
   gZapJournal.display();
}

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, display, ())
{
   glFlush();
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
   glutSwapBuffers();
}

#include <stdio.h>
class StdoutLogConsumer : public LogConsumer
{
public:
   void logString(const char *string)
   {
      printf("%s\n", string);
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
      {
         fprintf(f, "%s\n", string);
         fflush(f);
      }
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
      GameConnection *theConnection = new GameConnection();
      gClientGame->setConnectionToServer(theConnection);

      const char *name = gNameEntryUserInterface.getText();
      if(!name[0])
         name = "Playa";

      theConnection->setClientName(name);
      theConnection->setSimulatedNetParams(gSimulatedPacketLoss, gSimulatedPing);

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
   NetClassRep::logBitUsage();
}

extern void InitController();

TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, startup, (bool hasClient, bool hasServer, bool connectLocal, bool connectRemote, bool nameSet))
{
   if(hasClient)
      gClientGame = new ClientGame(Address());

   if(hasServer)
      hostGame(hasClient == false, gBindAddress);
   else if(connectRemote)
      joinGame(gConnectAddress, false);

   if(!connectLocal && !connectRemote)
   {
      if(!nameSet)
         gNameEntryUserInterface.activate();
      else
         gMainMenuUserInterface.activate();
   }
}

};

using namespace Zap;

int main(int argc, char **argv)
{
   //TNLLogEnable(LogConnectionProtocol, true);
   //TNLLogEnable(LogNetConnection, true);
   TNLLogEnable(LogNetInterface, true);
   TNLLogEnable(LogPlatform, true);
   TNLLogEnable(LogNetBase, true);
   //TNLLogEnable(LogBlah, true);

   bool hasClient = true;
   bool hasServer = false;
   bool connectLocal = false;
   bool connectRemote = false;
   bool nameSet = false;

   U32 maxPlayers = 128;

   for(S32 i = 1; i < argc;i+=2)
   {
      bool hasAdditionalArg = (i != argc - 1);

      if(!stricmp(argv[i], "-crazybot"))
      {
         gIsCrazyBot = true;

         // Generate a name...
         char journalPath[512];
         getcwd(journalPath, 512);

         char *name = tmpnam(NULL);
         nameSet = true;
         gNameEntryUserInterface.setText(name);

         strcat(journalPath, name);
         strcat(journalPath, "journal");
         gZapJournal.record(journalPath);

         // Connect to specified server
         connectRemote = true;
         if(hasAdditionalArg)
            gConnectAddress.set(argv[i+1]);

      }
      else if(!stricmp(argv[i], "-server"))
      {
         hasServer = true;
         connectLocal = true;
         if(hasAdditionalArg)
            gBindAddress.set(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-connect"))
      {
         connectRemote = true;
         if(hasAdditionalArg)
            gConnectAddress.set(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-master"))
      {
         if(hasAdditionalArg)
            gMasterAddressString = argv[i+1];
      }
      else if(!stricmp(argv[i], "-joystick"))
      {
         OptionsMenuUserInterface::joystickEnabled = true;
         if(hasAdditionalArg)
            gJoystickType = atoi(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-loss"))
      {
         if(hasAdditionalArg)
            gSimulatedPacketLoss = atof(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-lag"))
      {
         if(hasAdditionalArg)
            gSimulatedPing = atoi(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-jsave"))
      {
         if(hasAdditionalArg)
            gZapJournal.record(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-jplay"))
      {
         if(hasAdditionalArg)
            gZapJournal.load(argv[i+1]);
      }
      else if(!stricmp(argv[i], "-dedicated"))
      {
         hasClient = false;
         hasServer = true;
         if(hasAdditionalArg)
            gBindAddress.set(argv[i+1]);
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
   gZapJournal.startup(hasClient, hasServer, connectLocal, connectRemote, nameSet);

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

      atexit(onExit);

      glutMainLoop();
   }
   else
      dedicatedServerLoop();
   return 0;
}