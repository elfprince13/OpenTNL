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

#include "gameObject.h"
#include "glutInclude.h"

using namespace TNL;

namespace Zap
{

GameObject::GameObject()
{
   mGame = NULL;
   mLastQueryId = 0;
   mObjectTypeMask = UnknownType;
   mDisableCollisionCount = 0;
}

void GameObject::setControllingClient(GameConnection *c)
{
   mControllingClient = c;
}

Point GameObject::getRenderPos()
{
   return Point();
}

Point GameObject::getActualPos()
{
   return Point();
}

void GameObject::setActualPos(Point p)
{
}

F32 GameObject::getUpdatePriority(NetObject *scopeObject, U32 updateMask, S32 updateSkips)
{
   GameObject *so = (GameObject *) scopeObject;

   Point center = so->extent.getCenter();  

   Point nearest;
   if(center.x < extent.min.x)
      nearest.x = extent.min.x;
   else if(center.x > extent.max.x)
      nearest.x = extent.max.x;
   else
      nearest.x = center.x;

   if(center.y < extent.min.y)
      nearest.y = extent.min.y;
   else if(center.y > extent.max.y)
      nearest.y = extent.max.y;
   else
      nearest.y = center.y;

   F32 distance = (nearest - center).len();

   return (200 / distance) + updateSkips;
}

void GameObject::damageObject(DamageInfo *theInfo)
{

}

GameConnection *GameObject::getControllingClient()
{
   return mControllingClient;
}

void GameObject::setExtent(Rect &extents)
{
   if(mGame)
   {
      // remove from the extents database for current extents
      mGame->getGridDatabase()->removeFromExtents(this, extent);
      // and readd for the new extent
      mGame->getGridDatabase()->addToExtents(this, extents);
   }
   extent = extents;
}

void GameObject::findObjects(U32 typeMask, Vector<GameObject *> &fillVector, Rect &ext)
{
   if(!mGame)
      return;
   mGame->getGridDatabase()->findObjects(typeMask, fillVector, ext);
}

GameObject *GameObject::findObjectLOS(U32 typeMask, U32 stateIndex, Point rayStart, Point rayEnd, float &collisionTime)
{
   if(!mGame)
      return NULL;
   return mGame->getGridDatabase()->findObjectLOS(typeMask, stateIndex, rayStart, rayEnd, collisionTime);
}

void GameObject::addToGame(Game *theGame)
{
   TNLAssert(mGame == NULL, "Error, already in a game.");
   theGame->addToGameObjectList(this);
   mGame = theGame;
   mGame->getGridDatabase()->addToExtents(this, extent);
   onAddedToGame(theGame);
}

void GameObject::onAddedToGame(Game *)
{
}

void GameObject::removeFromGame()
{
   if(mGame)
   {
      mGame->getGridDatabase()->removeFromExtents(this, extent);
      mGame->removeFromGameObjectList(this);
      mGame = NULL;
   }
}

bool GameObject::getCollisionPoly(Vector<Point> &polyPoints)
{
   return false;
}

bool GameObject::getCollisionCircle(U32 stateIndex, Point &point, float &radius)
{
   return false;
}

void GameObject::render()
{
}

void GameObject::processServerMove(Move *)
{
}

void GameObject::processClientMove(Move *, bool replay)
{
}

void GameObject::processClient(U32 deltaT)
{
}

void GameObject::processServer(U32 deltaT)
{
}

void GameObject::writeControlState(BitStream *)
{
}

void GameObject::readControlState(BitStream *)
{
}

void GameObject::processArguments(S32 argc, const char**argv)
{
}

bool GameObject::onGhostAdd(GhostConnection *theConnection)
{
   addToGame(gClientGame);
   return true;
}

};
