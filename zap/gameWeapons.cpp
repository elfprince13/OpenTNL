
#include "gameWeapons.h"

// do not add a weapon with a fire delay > Ship::MaxFireDelay
// or update the constant.

ShipWeaponInfo gWeapons[] =
{
   {"Phaser",   100,  500,  500},
   {"Grenade",  700,  5000, 5000},
   {"Bouncer",  100,  2500, 2500},
   {"Triple",   200,  2500, 2500},
   {NULL, 0, 0, 0},
   {NULL, 0, 0, 0}
};

