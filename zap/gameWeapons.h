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

#ifndef _GAMEWEAPONS_H_
#define _GAMEWEAPONS_H_

#include "tnlPlatform.h"

using namespace TNL;

enum WeaponTypes
{
   WeaponPhaser = 0,
   WeaponGrenade,
   WeaponBounce,
   WeaponTriple,
   WeaponBurst,
   WeaponMineLayer,
   WeaponCount,
};

enum WeaponConsts
{
   MaxFireDelay = 2048,
};

struct ShipWeaponInfo
{
   const char *name; // Display name of the weapon.
   U32 fireDelay;    // Delay between shots.
   U32 minEnergy;    // Minimum energy to fire.
   U32 drainEnergy;  // Amount of energy to drain per shot.
};

extern ShipWeaponInfo gWeapons[WeaponCount];

#endif