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
#include "timer.h"
#include "sfx.h"
#include "voiceCodec.h"

namespace Zap
{

class GameType : public GameObject
{
public:
   enum
   {
      RespawnDelay = 1500,
   };

   struct ClientRef
   {
      StringTableEntry name;  /// Name of client - guaranteed to be unique of current clients
      S32 teamId;
      S32 score;
      Timer respawnTimer;

      bool wantsScoreboardUpdates;
      SafePtr<GameConnection> clientConnection;
      RefPtr<SFXObject> voiceSFX;
      RefPtr<VoiceDecoder> decoder;

      U32 ping;
      ClientRef() { ping = 0; score = 0; }
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
   U32 mThisClientName; ///< Set to the client name of this client (only on the ghost of the GameType)
   Timer mScoreboardUpdateTimer;
   Timer mGameTimer;
   Timer mGameTimeUpdateTimer;
   S32 mTeamScoreLimit;
   bool mGameOver; // set to true when an end condition is met

   enum {
      MaxPing = 999,
      DefaultGameTime = 20 * 60 * 1000,
      DefaultTeamScoreLimit = 8,
   };

   static Vector<RangedU32<0, MaxPing> > mPingTimes; ///< Static vector used for constructing update RPCs
   static Vector<Int<24> > mScores;

   GameType();
   void countTeamPlayers();

   Color getClientColor(const StringTableEntry &clientName)
   {
      S32 index = findClientIndexByName(clientName);
      if(index != -1)
         return mTeams[mClientList[index].teamId].color;
      return Color();
   }

   S32 findClientIndexByName(const StringTableEntry &name);
   S32 findClientIndexByConnection(GameConnection *theConnection);

   void processArguments(S32 argc, const char **argv);
   virtual bool processLevelItem(S32 argc, const char **argv);
   void onAddedToGame(Game *theGame);
   void onGhostAvailable(GhostConnection *theConnection);

   void idle(GameObject::IdleCallPath path);

   void setTeamScore(S32 teamIndex, S32 newScore);
   virtual void gameOverManGameOver();

   virtual void serverAddClient(GameConnection *theClient);
   virtual void serverRemoveClient(GameConnection *theClient);

   virtual bool objectCanDamageObject(GameObject *damager, GameObject *victim) { return true; }
   virtual void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);
   virtual void controlObjectForClientRemoved(GameConnection *theClient, GameObject *clientObject);

   virtual void spawnShip(GameConnection *theClient);

   virtual void addClientGameMenuOptions(Vector<const char *> &menuOptions);
   virtual void processClientGameMenuOption(U32 index);

   virtual void renderInterfaceOverlay(bool scoreboardVisible);
   void renderTimeLeft();
   void renderTalkingClients();
   virtual void updateClientScoreboard(S32 clientIndex);

   virtual void performProxyScopeQuery(GameObject *scopeObject, GameConnection *connection);

   TNL_DECLARE_RPC(s2cAddTeam, (StringTableEntryRef teamName, F32 r, F32 g, F32 b));
   TNL_DECLARE_RPC(s2cSetTeamScore, (U32 teamIndex, U32 score));
   TNL_DECLARE_RPC(s2cAddClient, (StringTableEntryRef clientName, bool isMyClient));
   TNL_DECLARE_RPC(s2cRemoveClient, (StringTableEntryRef clientName));

   TNL_DECLARE_RPC(c2sChangeTeams, ());
   TNL_DECLARE_RPC(s2cClientJoinedTeam, (StringTableEntryRef clientName, U32 teamIndex));

   void sendChatDisplayEvent(S32 clientIndex, bool global, NetEvent *theEvent);
   TNL_DECLARE_RPC(c2sSendChat, (bool global, const char *message));
   TNL_DECLARE_RPC(c2sSendChatSTE, (bool global, StringTableEntryRef ste));
   TNL_DECLARE_RPC(s2cDisplayChatMessage, (bool global, StringTableEntryRef clientName, const char *message));
   TNL_DECLARE_RPC(s2cDisplayChatMessageSTE, (bool global, StringTableEntryRef clientName, StringTableEntryRef message));

   TNL_DECLARE_RPC(s2cKillMessage, (StringTableEntryRef victim, StringTableEntryRef killer));
   TNL_DECLARE_RPC(s2cScoreboardUpdate, (const Vector<RangedU32<0, MaxPing> > &pingTimes, const Vector<Int<24> > &scores));

   TNL_DECLARE_RPC(s2cSetGameOver, (bool gameOver));
   TNL_DECLARE_RPC(s2cSetTimeRemaining, (U32 timeLeft));
   TNL_DECLARE_RPC(c2sRequestScoreboardUpdates, (bool updates));

   TNL_DECLARE_RPC(c2sVoiceChat, (bool echo, ByteBufferRef compressedVoice));
   TNL_DECLARE_RPC(s2cVoiceChat, (StringTableEntryRef client, ByteBufferRef compressedVoice));

   TNL_DECLARE_CLASS(GameType);
};

};

#endif