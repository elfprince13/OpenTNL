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

#include "UIQueryServers.h"
#include "UIMenus.h"
#include "tnlRandom.h"
#include "masterConnection.h"
#include "gameNetInterface.h"
#include "glutInclude.h"
#include "game.h"

namespace Zap
{

QueryServersUserInterface gQueryServersUserInterface;

void QueryServersUserInterface::onActivate()
{
   servers.clear();
   sortColumn = 0;
   pendingPings = 0;
   pendingQueries = 0;
   sortAscending = true;
   mNonce.getRandom();

   logprintf("pinging broadcast servers...");
   Address broadcastAddress(IPProtocol, Address::Broadcast, 28000);
   broadcastPingSendTime = Platform::getRealMilliseconds();

   gClientGame->getNetInterface()->sendPing(broadcastAddress, mNonce);
   if(gClientGame->getConnectionToMaster())
      gClientGame->getConnectionToMaster()->startGameTypesQuery();
}

void QueryServersUserInterface::addPingServers(const Vector<IPAddress> &ipList)
{
   for(S32 i = 0; i < ipList.size(); i++)
   {
      ServerRef s;
      s.state = ServerRef::Start;
      s.id = ++lastUsedServerId;
      s.sendNonce.getRandom();
      s.serverAddress.set(ipList[i]);
      s.sendCount = 0;
      s.pingTime = 9999;
      s.playerCount = s.maxPlayers = -1;
      s.isFromMaster = true;
      strcpy(s.serverName, "Internet Server");
      servers.push_back(s);
   }
}

void QueryServersUserInterface::gotPingResponse(const Address &theAddress, const Nonce &theNonce, U32 clientIdentityToken)
{
   // see if this ping is a server from the local broadcast ping:
   if(mNonce == theNonce)
   {
      for(S32 i = 0; i < servers.size(); i++)
         if(servers[i].sendNonce == theNonce && servers[i].serverAddress == theAddress)
            return;

      // it was from a local ping
      ServerRef s;
      s.pingTime = Platform::getRealMilliseconds() - broadcastPingSendTime;
      s.state = ServerRef::ReceivedPing;
      s.id = ++lastUsedServerId;
      s.sendNonce = theNonce;
      s.identityToken = clientIdentityToken;
      s.serverAddress = theAddress;
      s.sendCount = 0;
      s.playerCount = -1;
      s.maxPlayers = -1;
      s.isFromMaster = false;
      strcpy(s.serverName, "LAN Server");
      servers.push_back(s);
      return;
   }

   // see if this ping is in the list:
   for(S32 i = 0; i < servers.size(); i++)
   {
      ServerRef &s = servers[i];
      if(s.sendNonce == theNonce && s.serverAddress == theAddress &&
            s.state == ServerRef::SentPing)
      {
         s.pingTime = Platform::getRealMilliseconds() - s.lastSendTime;
         s.state = ServerRef::ReceivedPing;
         s.identityToken = clientIdentityToken;
         pendingPings--;
         break;
      }
   }
   shouldSort = true;
}

void QueryServersUserInterface::gotQueryResponse(const Address &theAddress, const Nonce &clientNonce, const char *serverName, U32 playerCount, U32 maxPlayers)
{
   for(S32 i = 0; i < servers.size(); i++)
   {
      ServerRef &s = servers[i];
      if(s.sendNonce == clientNonce && s.serverAddress == theAddress &&
            s.state == ServerRef::SentQuery)
      {
         s.playerCount = playerCount;
         s.maxPlayers = maxPlayers;
         dSprintf(s.serverName, sizeof(s.serverName), "%s", serverName);
         s.state = ServerRef::ReceivedQuery;
         pendingQueries--;
      }
   }
   shouldSort = true;
   // find this 
}

void QueryServersUserInterface::idle(U32 t)
{
   U32 time = Platform::getRealMilliseconds();

   for(S32 i = 0; i < servers.size(); i++)
   {
      ServerRef &s = servers[i];

      if(s.state == ServerRef::SentPing && (time - s.lastSendTime) > PingQueryTimeout)
      {
         s.state = ServerRef::Start;
         pendingPings--;
      }
      else if(s.state == ServerRef::SentQuery && (time - s.lastSendTime) > PingQueryTimeout)
      {
         s.state = ServerRef::ReceivedPing;
         pendingQueries--;
      }
   }
   if(pendingPings < MaxPendingPings)
   {
      for(S32 i = 0; i < servers.size() ; i++)
      {
         ServerRef &s = servers[i];
         if(s.state == ServerRef::Start)
         {
            s.sendCount++;
            if(s.sendCount > PingQueryRetryCount)
            {
               s.pingTime = 999;
               strcpy(s.serverName, "PingTimedOut");
               s.playerCount = 0;
               s.maxPlayers = 0;
               s.state = ServerRef::ReceivedQuery;
               shouldSort = true;
            }
            else
            {
               s.state = ServerRef::SentPing;
               s.lastSendTime = time;
               s.sendNonce.getRandom();
               gClientGame->getNetInterface()->sendPing(s.serverAddress, s.sendNonce);
               pendingPings++;
               if(pendingPings >= MaxPendingPings)
                  break;
            }
         }
      }
   }
   if(pendingPings == 0 && (pendingQueries < MaxPendingQueries))
   {
      for(S32 i = 0; i < servers.size(); i++)
      {
         ServerRef &s = servers[i];
         if(s.state == ServerRef::ReceivedPing)
         {
            s.sendCount++;
            if(s.sendCount > PingQueryRetryCount)
            {
               strcpy(s.serverName, "QueryTimedOut");
               s.playerCount = s.maxPlayers = 0;
               s.state = ServerRef::ReceivedQuery;
               shouldSort = true;
            }
            else
            {
               s.state = ServerRef::SentQuery;
               s.lastSendTime = time;
               gClientGame->getNetInterface()->sendQuery(s.serverAddress, s.sendNonce, s.identityToken);
               pendingQueries++;
               if(pendingQueries >= MaxPendingQueries)
                  break;  
            }
         }
      }
   }
}

QueryServersUserInterface::QueryServersUserInterface()
{
   /*
   for(U32 i = 0;i < 512; i++)
   {
      ServerRef s;
      dSprintf(s.serverName, MaxServerNameLen, "Svr%8x", Random::readI());
      s.id = i;
      s.pingTime = Random::readF() * 512;
      s.serverAddress.port = 28000;
      s.serverAddress.netNum[0] = Random::readI();
      s.maxPlayers = Random::readF() * 16 + 8;
      s.playerCount = Random::readF() * s.maxPlayers;
      servers.push_back(s);
   }*/
   lastUsedServerId = 0;
   sortColumn = 0;
   lastSortColumn = 0;
   sortAscending = true;

   columns.push_back(ColumnInfo("SERVER NAME", 3));
   columns.push_back(ColumnInfo("PING", 300));
   columns.push_back(ColumnInfo("PLAYERS", 420));
   columns.push_back(ColumnInfo("ADDRESS", 550));

   selectedId = 0xFFFFFF;

   sort();
   shouldSort = false;
}

S32 QueryServersUserInterface::findSelectedIndex()
{
   for(S32 i = 0; i < servers.size(); i++)
      if(servers[i].id == selectedId)
         return i;
   return -1;
}

void QueryServersUserInterface::render()
{
   if(shouldSort)
   {
      shouldSort = false;
      sort();
   }
   glViewport(0, 0, windowWidth, windowHeight);

   glClearColor(0, 0, 0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glColor3f(1,1,1);

   drawCenteredString(0, 25, "CHOOSE A SERVER TO JOIN:");
   drawCenteredString(560, 18, "UP, DOWN, PAGEUP, PAGEDOWN to select, ENTER to join.");
   drawCenteredString(580, 18, "LEFT, RIGHT select sort column, SPACE to sort.  ESC exits.");

   for(S32 i = 0; i < columns.size(); i++)
   {
      drawString(columns[i].xStart, 35, 24, columns[i].name);      
   }

   S32 x1 = columns[sortColumn].xStart - 2;
   S32 x2;
   if(sortColumn == columns.size() - 1)
      x2 = 799;
   else
      x2 = columns[sortColumn+1].xStart - 6;

   glBegin(GL_LINE_LOOP);
   glVertex2f(x1, 33);
   glVertex2f(x2, 33);
   glVertex2f(x2, 61);
   glVertex2f(x1, 61);
   glEnd();

   if(servers.size())
   {
      S32 selectedIndex = findSelectedIndex();
      if(selectedIndex == -1)
         selectedIndex = 0;

      S32 firstServer = selectedIndex - ServersAboveBelow;
      S32 lastServer = selectedIndex + ServersAboveBelow;

      if(firstServer < 0)
      {
         lastServer -= firstServer;
         firstServer = 0;
      }
      if(lastServer >= servers.size())
      {
         lastServer = servers.size() - 1;
      }

      for(S32 i = firstServer; i <= lastServer; i++)
      {
         U32 y = 65 + (i - firstServer) * 23;
         U32 fontSize = 20;

         if(i == selectedIndex)
         {
            glColor3f(0,0,0.4);
            glBegin(GL_POLYGON);
            glVertex2f(0, y);
            glVertex2f(799, y);
            glVertex2f(799, y + 22);
            glVertex2f(0, y + 22);
            glEnd();
            glColor3f(0,0,1.0);
            glBegin(GL_LINE_LOOP);
            glVertex2f(0, y);
            glVertex2f(799, y);
            glVertex2f(799, y + 22);
            glVertex2f(0, y + 22);
            glEnd();
         }

         glColor3f(1,1,1);
         drawString(columns[0].xStart, y, fontSize, servers[i].serverName);

         if(servers[i].pingTime < 100)
            glColor3f(0,1,0);
         else if(servers[i].pingTime < 250)
            glColor3f(1,1,0);
         else
            glColor3f(1,0,0);
         drawStringf(columns[1].xStart, y, fontSize, "%d", servers[i].pingTime);

         if(servers[i].playerCount == servers[i].maxPlayers)
            glColor3f(1,0,0);
         else if(servers[i].playerCount == 0)
            glColor3f(1,1,0);
         else
            glColor3f(0,1,0);
         if(servers[i].playerCount < 0)
            drawString(columns[2].xStart, y, fontSize, "?? / ??");
         else
            drawStringf(columns[2].xStart, y, fontSize, "%d / %d", servers[i].playerCount, servers[i].maxPlayers);
         glColor3f(1,1,1);
         drawString(columns[3].xStart, y, fontSize, servers[i].serverAddress.toString());
      }
   }
   glColor3f(0.7, 0.7, 0.7);
   for(S32 i = 1; i < columns.size(); i++)
   {
      glBegin(GL_LINES);
      glVertex2f(columns[i].xStart - 4, 35);
      glVertex2f(columns[i].xStart - 4, 550);
      glEnd();
   }
   glBegin(GL_LINES);
   glVertex2f(0, 62);
   glVertex2f(800, 62);
   glEnd();
}

void QueryServersUserInterface::onKeyDown(U32 key)
{
   switch(key)
   {
      case ' ':
         if(lastSortColumn == sortColumn)
            sortAscending = !sortAscending;
         else
         {
            lastSortColumn = sortColumn;
            sortAscending = true;
         }
         sort();
         break;
      case '\r':
         {
            S32 currentIndex = findSelectedIndex();
            if(currentIndex == -1)
               currentIndex = 0;

            if(servers.size() > currentIndex)
            {
               // join the selected game
               joinGame(servers[currentIndex].serverAddress, servers[currentIndex].isFromMaster, false);

               // and clear out the servers, so that we don't do any more pinging
               servers.clear();
            }
         }
         break;
      case 27:
         gMainMenuUserInterface.activate();
         break;
   }
}

void QueryServersUserInterface::onSpecialKeyDown(U32 key)
{
   if(!servers.size())
      return;

   S32 currentIndex = findSelectedIndex();
   if(currentIndex == -1)
      currentIndex = 0;

   switch(key)
   {
      case GLUT_KEY_PAGE_UP:
         currentIndex -= ServersPerScreen - 1;
         break;
      case GLUT_KEY_PAGE_DOWN:
         currentIndex += ServersPerScreen - 1;
         break;
      case GLUT_KEY_UP:
         currentIndex--;
         break;
      case GLUT_KEY_DOWN:
         currentIndex++;
         break;
      case GLUT_KEY_LEFT:
         sortColumn--;
         if(sortColumn < 0)
            sortColumn = 0;
         break;
      case GLUT_KEY_RIGHT:
         sortColumn++;
         if(sortColumn >= columns.size())
            sortColumn = columns.size() - 1;
         break;
   }
   if(currentIndex < 0)
      currentIndex = 0;
   if(currentIndex >= servers.size())
      currentIndex = servers.size() - 1;

   selectedId = servers[currentIndex].id;
}

static S32 QSORT_CALLBACK compareFuncName(const void *a, const void *b)
{
   return stricmp(((QueryServersUserInterface::ServerRef *) a)->serverName,
                  ((QueryServersUserInterface::ServerRef *) b)->serverName);
}

static S32 QSORT_CALLBACK compareFuncPing(const void *a, const void *b)
{
   return S32(((QueryServersUserInterface::ServerRef *) a)->pingTime -
          ((QueryServersUserInterface::ServerRef *) b)->pingTime);
}

static S32 QSORT_CALLBACK compareFuncPlayers(const void *a, const void *b)
{
   S32 pc = S32(((QueryServersUserInterface::ServerRef *) a)->playerCount -
          ((QueryServersUserInterface::ServerRef *) b)->playerCount);
   if(pc)
      return pc;

   return S32(((QueryServersUserInterface::ServerRef *) a)->maxPlayers -
          ((QueryServersUserInterface::ServerRef *) b)->maxPlayers);
}

static S32 QSORT_CALLBACK compareFuncAddress(const void *a, const void *b)
{
   return S32(((QueryServersUserInterface::ServerRef *) a)->serverAddress.netNum[0] -
          ((QueryServersUserInterface::ServerRef *) b)->serverAddress.netNum[0]);
}

void QueryServersUserInterface::sort()
{
   switch(sortColumn)
   {
      case 0:
         qsort(servers.address(), servers.size(), sizeof(ServerRef), compareFuncName);
         break;
      case 1:
         qsort(servers.address(), servers.size(), sizeof(ServerRef), compareFuncPing);
         break;
      case 2:
         qsort(servers.address(), servers.size(), sizeof(ServerRef), compareFuncPlayers);
         break;
      case 3:
         qsort(servers.address(), servers.size(), sizeof(ServerRef), compareFuncAddress);
         break;
   }
   if(!sortAscending)
   {
      S32 size = servers.size() / 2;
      S32 totalSize = servers.size();

      for(S32 i = 0; i < size; i++)
      {
         ServerRef temp = servers[i];
         servers[i] = servers[totalSize - i - 1];
         servers[totalSize - i - 1] = temp;
      }
   }
}

};