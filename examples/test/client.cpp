//-----------------------------------------------------------------------------
// Torque Network Library Test Program (Dedicated Server)
// 
// Copyright (c) 2004 GarageGames.Com
//-----------------------------------------------------------------------------

#include "testGame.h"
#include <stdio.h>
#include <string.h>

class DedicatedServerLogConsumer : public TNL::LogConsumer
{
public:
   void logString(const char *string)
   {
      printf("%s\n", string);
   }
} gDedicatedServerLogConsumer;

int main(int argc, const char **argv)
{
   const char *localBroadcastAddress = "IP:broadcast:28999";

   TNL::S32 port = 28999;
   if(argc == 2)
      port = atoi(argv[1]);
	TestGame* theGame = new TestGame(false, TNL::Address(TNL::IPProtocol, TNL::Address::Any, port),TNL::Address(localBroadcastAddress));
   for(;;)
      theGame->tick();
}
