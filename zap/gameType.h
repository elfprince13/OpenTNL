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

#ifndef _GAMETYPE_H_
#define _GAMETYPE_H_

#include "gameObject.h"

namespace Zap
{

class GameType : public GameObject
{
public:
   struct ClientRef
   {
      U32 clientId;
      StringTableEntry name;
      S32 teamId;
      U32 score;
      bool wantsScoreboardUpdates;
      SafePtr<GameConnection> clientConnection;
      U32 ping;
   };

   Vector<ClientRef> mClientList;

   struct Team
   {
      StringTableEntry name;
      Color color;
      Vector<Point> spawnPoints;
      U32 numPlayers;
      U32 score;
      Team() { numPlayers = 0; score = 0; }
   };
   Vector<Team> mTeams;
   U32 mThisClientId; ///< Set to the client ID of this client (only on the ghost of the GameType)
   U32 mTimeUntilScoreboardUpdate;

   GameType();
   void countTeamPlayers();

   S32 findClientIndexById(U32 clientId);
   S32 findClientIndexByConnection(GameConnection *theConnection);

   void processArguments(S32 argc, const char **argv);
   virtual bool processLevelItem(S32 argc, const char **argv);
   void onAddedToGame(Game *theGame);
   void onGhostAvailable(GhostConnection *theConnection);
   void processServer(U32 deltaT);

   virtual void serverAddClient(GameConnection *theClient);
   virtual void serverRemoveClient(GameConnection *theClient);

   virtual void controlObjectForClientKilled(GameConnection *theClient);
   virtual void spawnShip(GameConnection *theClient);

   virtual void addClientGameMenuOptions(Vector<const char *> &menuOptions);
   virtual void processClientGameMenuOption(U32 index);

   virtual void renderInterfaceOverlay(bool scoreboardVisible) {};
   virtual void updateClientScoreboard(S32 clientIndex);

   TNL_DECLARE_RPC(s2cAddTeam, (StringTableEntry teamName, F32 r, F32 g, F32 b));
   TNL_DECLARE_RPC(s2cSetTeamScore, (U32 teamIndex, U32 score));
   TNL_DECLARE_RPC(s2cAddClient, (U32 clientId, StringTableEntry clientName, bool isMyClient));
   TNL_DECLARE_RPC(s2cRemoveClient, (U32 clientId));

   TNL_DECLARE_RPC(c2sChangeTeams, ());
   TNL_DECLARE_RPC(s2cClientJoinedTeam, (U32 clientId, U32 teamIndex));

   TNL_DECLARE_RPC(c2sSendChat, (bool global, const char *message));
   TNL_DECLARE_RPC(s2cDisplayChatMessage, (bool global, StringTableEntry clientName, const char *message));

   TNL_DECLARE_RPC(c2sRequestScoreboardUpdates, (bool updates));

   TNL_DECLARE_CLASS(GameType);
};

};

#endif