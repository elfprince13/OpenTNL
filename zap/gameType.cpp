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

#include "gameType.h"
#include "ship.h"
#include "UIGame.h"
#include "gameNetInterface.h"

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(GameType);

GameType::GameType()
{
   mNetFlags.set(Ghostable);
   mTimeUntilScoreboardUpdate = 0;
}

void GameType::processArguments(S32 argc, const char **argv)
{

}

void GameType::processServer(U32 deltaT)
{
   if(deltaT > mTimeUntilScoreboardUpdate)
   {
      for(S32 i = 0; i < mClientList.size(); i++)
         mClientList[i].ping = (U32) mClientList[i].clientConnection->getRoundTripTime();

      for(S32 i = 0; i < mClientList.size(); i++)
         if(mClientList[i].wantsScoreboardUpdates)
            updateClientScoreboard(i);

      mTimeUntilScoreboardUpdate = 1000;
   }
   else
      mTimeUntilScoreboardUpdate -= deltaT;
}

void GameType::onAddedToGame(Game *theGame)
{
   theGame->setGameType(this);
   setInterface(theGame->getNetInterface());
   if(!isGhost())
      setScopeAlways();
}

bool GameType::processLevelItem(S32 argc, const char **argv)
{
   if(!stricmp(argv[0], "Team"))
   {
      if(argc < 5)
         return false;
      Team t;
      t.numPlayers = 0;

      t.name.set(argv[1]);
      t.color.read(argv + 2);
      mTeams.push_back(t);
   }
   else if(!stricmp(argv[0], "Spawn"))
   {
      if(argc < 4)
         return false;
      S32 teamIndex = atoi(argv[1]);
      Point p;
      p.read(argv + 2);
      p *= getGame()->getGridSize();
      if(teamIndex >= 0 && teamIndex < mTeams.size())
         mTeams[teamIndex].spawnPoints.push_back(p);
   }
   else
      return false;
   return true;
}

S32 GameType::findClientIndexByConnection(GameConnection *theConnection)
{
   for(S32 clientIndex = 0; clientIndex < mClientList.size(); clientIndex++)
      if(mClientList[clientIndex].clientConnection == theConnection)
         return clientIndex;
   return -1;
}

S32 GameType::findClientIndexById(U32 clientId)
{
   for(S32 clientIndex = 0; clientIndex < mClientList.size(); clientIndex++)
      if(mClientList[clientIndex].clientId == clientId)
         return clientIndex;
   return -1;
}

void GameType::spawnShip(GameConnection *theClient)
{
   S32 clientIndex = findClientIndexByConnection(theClient);
   S32 teamIndex = mClientList[clientIndex].teamId;

   Point spawnPoint;
   S32 spawnIndex = Random::readI() % mTeams[teamIndex].spawnPoints.size();
   spawnPoint = mTeams[teamIndex].spawnPoints[spawnIndex];

   Ship *newShip = new Ship(mClientList[clientIndex].name, spawnPoint, mTeams[teamIndex].color);
   newShip->addToGame(getGame());
   theClient->setControlObject(newShip);
}

void GameType::countTeamPlayers()
{
   for(S32 i = 0; i < mTeams.size(); i ++)
      mTeams[i].numPlayers = 0;

   for(S32 i = 0; i < mClientList.size(); i++)
      mTeams[mClientList[i].teamId].numPlayers++;
}

void GameType::serverAddClient(GameConnection *theClient)
{
   ClientRef cref;
   cref.clientId = theClient->mClientId;
   cref.name = theClient->playerName;

   cref.clientConnection = theClient;
   cref.score = 0;
   cref.wantsScoreboardUpdates = false;

   countTeamPlayers();

   U32 minPlayers = mTeams[0].numPlayers;
   S32 minTeamIndex = 0;

   for(S32 i = 1; i < mTeams.size(); i++)
   {
      if(mTeams[i].numPlayers < minPlayers)
      {
         minTeamIndex = i;
         minPlayers = mTeams[i].numPlayers;
      }
   }
   cref.teamId = minTeamIndex;
   mClientList.push_back(cref);

   s2cAddClient(cref.clientId, cref.name, false);
   s2cClientJoinedTeam(cref.clientId, cref.teamId);
   spawnShip(theClient);
}

void GameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   spawnShip(theClient);
}

void GameType::controlObjectForClientRemoved(GameConnection *theClient, GameObject *clientObject)
{

}

void GameType::addClientGameMenuOptions(Vector<const char *> &menuOptions)
{
   if(mTeams.size() > 1)
      menuOptions.push_back("CHANGE TEAMS");
}

void GameType::processClientGameMenuOption(U32 index)
{
   if(index == 0)
      c2sChangeTeams();
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sChangeTeams, (),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   if(mTeams.size() <= 1)
      return;

   GameConnection *source = (GameConnection *) NetObject::getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);

   // destroy the old ship
   GameObject *co = source->getControlObject();
   controlObjectForClientRemoved(source, co);
   if(co)
      getGame()->deleteObject(co, 0);

   U32 newTeamId = (mClientList[clientIndex].teamId + 1) % mTeams.size();
   mClientList[clientIndex].teamId = newTeamId;
   s2cClientJoinedTeam(mClientList[clientIndex].clientId, newTeamId);
   spawnShip(source);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cAddClient, (U32 id, StringTableEntry name, bool isMyClient),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   ClientRef cref;
   cref.clientId = id;
   cref.name = name;
   cref.teamId = 0;
   cref.wantsScoreboardUpdates = false;
   cref.ping = 0;
   mClientList.push_back(cref);

   if(isMyClient)
      mThisClientId = id;
   gGameUserInterface.displayMessage(Color(0.6f, 0.6f, 0.8f), "%s joined the game.", name.getString());
}

void GameType::serverRemoveClient(GameConnection *theClient)
{
   S32 clientIndex = findClientIndexByConnection(theClient);
   mClientList.erase(clientIndex);

   GameObject *theControlObject = theClient->getControlObject();
   controlObjectForClientRemoved(theClient, theControlObject);
   if(theControlObject)
      getGame()->deleteObject(theControlObject, 0);

   s2cRemoveClient(theClient->mClientId);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cRemoveClient, (U32 id),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexById(id);
   mClientList.erase(clientIndex);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cAddTeam, (StringTableEntry teamName, F32 r, F32 g, F32 b),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   Team team;
   team.name = teamName;
   team.color.r = r;
   team.color.g = g;
   team.color.b = b;
   mTeams.push_back(team);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cSetTeamScore, (U32 teamIndex, U32 score),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   mTeams[teamIndex].score = score;
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cClientJoinedTeam, (U32 clientId, U32 teamIndex),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexById(clientId);
   mClientList[clientIndex].teamId = teamIndex;
   gGameUserInterface.displayMessage(Color(0.6f, 0.6f, 0.8f), "%s joined team %s.", mClientList[clientIndex].name.getString(), mTeams[teamIndex].name.getString());
}

void GameType::onGhostAvailable(GhostConnection *theConnection)
{
   NetObject::setRPCDestConnection(theConnection);

   for(S32 i = 0; i < mTeams.size(); i++)
   {
      s2cAddTeam(mTeams[i].name, mTeams[i].color.r, mTeams[i].color.g, mTeams[i].color.b);
      s2cSetTeamScore(i, mTeams[i].score);
   }

   // add all the client and team information
   for(S32 i = 0; i < mClientList.size(); i++)
   {
      s2cAddClient(mClientList[i].clientId, mClientList[i].name, mClientList[i].clientConnection == theConnection);
      s2cClientJoinedTeam(mClientList[i].clientId, mClientList[i].teamId);
   }
   NetObject::setRPCDestConnection(NULL);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sSendChat, (bool global, const char *message),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   GameConnection *source = (GameConnection *) getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);

   RefPtr<NetEvent> theEvent = TNL_RPC_CONSTRUCT_NETEVENT(this, 
      s2cDisplayChatMessage, (global, source->playerName, message));

   S32 teamId = 0;
   
   if(!global)
      teamId = mClientList[clientIndex].teamId;

   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(global || mClientList[i].teamId == teamId)
         mClientList[i].clientConnection->postNetEvent(theEvent);
   }
}

extern Color gGlobalChatColor;
extern Color gTeamChatColor;

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cDisplayChatMessage, (bool global, StringTableEntry clientName, const char *message),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   Color theColor = global ? gGlobalChatColor : gTeamChatColor;

   gGameUserInterface.displayMessage(theColor, "%s: %s", clientName.getString(), message);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sRequestScoreboardUpdates, (bool updates),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   GameConnection *source = (GameConnection *) getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);
   mClientList[clientIndex].wantsScoreboardUpdates = updates;
   if(updates)
      updateClientScoreboard(clientIndex);
}

void GameType::updateClientScoreboard(S32 clientIndex)
{
}

};