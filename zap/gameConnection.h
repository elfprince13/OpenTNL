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

#ifndef _GAMECONNECTION_H_
#define _GAMECONNECTION_H_

#include "controlObjectConnection.h"
#include "sfx.h"

namespace Zap
{

static char *gConnectStatesTable[] = {
      "Not connected...",
      "Sending challenge request.",
      "Punching through firewalls.",
      "Computing puzzle solution.",
      "Sent connect request.",
      "Connection timed out.",
      "Connection rejected.",
      "Connected.",
      "Disconnected.",
      "Connection timed out.",
      ""
};

class ClientRef;

class GameConnection : public ControlObjectConnection
{
   typedef ControlObjectConnection Parent;

   // The server maintains a linked list of clients...
   GameConnection *mNext;
   GameConnection *mPrev;
   static GameConnection gClientList;

   bool mInCommanderMap;
   StringTableEntry mClientName;
   Vector<U32> mLoadout;
   SafePtr<ClientRef> mClientRef;

   void linkToClientList();
public:
   enum MessageColors
   {
      ColorWhite,
      ColorRed,
      ColorGreen,
      ColorBlue,
      ColorAqua,
      ColorYellow,
      ColorNuclearGreen,
      ColorCount,
   };

   GameConnection();
   ~GameConnection();

   void setClientName(const char *string) { mClientName = string; }
   void setClientRef(ClientRef *theRef);
   ClientRef *getClientRef();

   StringTableEntryRef getClientName() { return mClientName; }

   bool isInCommanderMap() { return mInCommanderMap; }
   TNL_DECLARE_RPC(c2sRequestCommanderMap, ());
   TNL_DECLARE_RPC(c2sReleaseCommanderMap, ());

   TNL_DECLARE_RPC(c2sRequestLoadout, (const Vector<U32> &loadout));
   TNL_DECLARE_RPC(c2sRequestEngineerBuild, (U32 buildObject));

   TNL_DECLARE_RPC(s2cDisplayMessageESI, (RangedU32<0, ColorCount> color, RangedU32<0, NumSFXBuffers> sfx, StringTableEntryRef formatString,
                   const Vector<StringTableEntry> &e, const Vector<const char *> &s, const Vector<S32> &i));
   TNL_DECLARE_RPC(s2cDisplayMessageE, (RangedU32<0, ColorCount> color, RangedU32<0, NumSFXBuffers> sfx, StringTableEntryRef formatString,
                   const Vector<StringTableEntry> &e));
   TNL_DECLARE_RPC(s2cDisplayMessage, (RangedU32<0, ColorCount> color, RangedU32<0, NumSFXBuffers> sfx, StringTableEntryRef formatString));

   static GameConnection *getClientList();
   GameConnection *getNextClient();

   const Vector<U32> &getLoadout() { return mLoadout; }
   void writeConnectRequest(BitStream *stream);
   bool readConnectRequest(BitStream *stream, const char **errorString);

   void onConnectionEstablished();

   void onConnectTerminated(TerminationReason r, const char *);

   void onConnectionTerminated(TerminationReason r, const char *string);

   TNL_DECLARE_NETCONNECTION(GameConnection);
};

};

#endif
