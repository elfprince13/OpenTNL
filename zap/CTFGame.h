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

#ifndef _CTFGAME_H_
#define _CTFGAME_H_

#include "gameType.h"
#include "item.h"

namespace Zap
{

class Ship;
class FlagItem;

class CTFGameType : public GameType
{
   typedef GameType Parent;
   enum Scores
   {
      KillScore    = 1,
      ReturnScore  = 1,
      CapScore     = 5,
      CapTeamScore = 2,

   };
public:
   void shipTouchFlag(Ship *theShip, FlagItem *theFlag);
   void flagDropped(const StringTableEntry &playerName, S32 flagTeamIndex);

   TNL_DECLARE_CLASS(CTFGameType);
};

};


#endif

