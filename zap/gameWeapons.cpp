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

#include "gameWeapons.h"
#include "projectile.h"

namespace Zap
{

// do not add a weapon with a fire delay > Ship::MaxFireDelay
// or update the constant.

ShipWeaponInfo gWeapons[] =
{
   {"Phaser",   100,  500,  500, 600, 1000, Projectile::Phaser },
   {"Bouncer",  100,  2500, 2500, 540, 1500, Projectile::Bounce },
   {"Triple",   200,  2500, 2500, 550, 850, Projectile::Triple },
   {"Burst",  700,  5000, 5000, 500, 1000, 0 },
   {NULL, 0, 0, 0, 0, 0, 0  },
   { "Turret", 0, 0, 0, 800, 800, Projectile::Turret },
};

void createWeaponProjectiles(U32 weapon, Point &dir, Point &shooterPos, Point &shooterVel, F32 shooterRadius, GameObject *shooter)
{
   GameObject *proj = NULL;
   ShipWeaponInfo *wi = gWeapons + weapon;
   Point projVel = dir * F32(wi->projVelocity) + dir * shooterVel.dot(dir);
   Point firePos = shooterPos + dir * shooterRadius;

   switch(weapon)
   {
      case WeaponTriple:
         {
            Point velPerp(projVel.y, -projVel.x);
            velPerp.normalize(50.0f);
            (new Projectile(wi->projectileType, firePos, projVel + velPerp, wi->projLiveTime, shooter))->addToGame(shooter->getGame());
            (new Projectile(wi->projectileType, firePos, projVel - velPerp, wi->projLiveTime, shooter))->addToGame(shooter->getGame());
         }
      case WeaponPhaser:
      case WeaponBounce:
      case WeaponTurretBlaster:
         (new Projectile(wi->projectileType, firePos, projVel, wi->projLiveTime, shooter))->addToGame(shooter->getGame());
         break;
      case WeaponBurst:
         (new GrenadeProjectile(firePos, projVel, wi->projLiveTime, shooter))->addToGame(shooter->getGame());
         break;
   }
}

};