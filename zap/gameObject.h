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

#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#include "point.h"
#include "gameConnection.h"
#include "../tnl/tnlNetObject.h"
#include "game.h"

namespace Zap
{

class GridDatabase;


enum GameObjectType
{
   UnknownType = 1 << 0,
   ShipType = 1 << 1,
   BarrierType = 1 << 2,
   MoveableType = 1 << 3,
   ProjectileType = 1 << 4,
   ItemType = 1 << 5,

   AllObjectTypes = 0xFFFFFFFF,
};

struct DamageInfo
{
   Point collisionPoint;
   Point impulseVector;
   float damageAmount;
   U32 damageType;
   GameObject *damagingObject;   
};

class GameObject : public NetObject
{
   friend class GridDatabase;

   typedef NetObject Parent;
   Game *mGame;
   U32 mLastQueryId;
   SafePtr<GameConnection> mControllingClient;
   U32 mDisableCollisionCount;

   Rect extent;
protected:
   U32 mObjectTypeMask;
public:

   GameObject();
   ~GameObject() { removeFromGame(); }

   void addToGame(Game *theGame);
   virtual void onAddedToGame(Game *theGame);
   void removeFromGame();

   Game *getGame() { return mGame; }

   void setExtent(Rect &extentRect);
   Rect getExtent() { return extent; }
   void findObjects(U32 typeMask, Vector<GameObject *> &fillVector, Rect &extents);
   GameObject *findObjectLOS(U32 typeMask, U32 stateIndex, Point rayStart, Point rayEnd, float &collisionTime);

   bool isControlled() { return mControllingClient.isValid(); }
   void setControllingClient(GameConnection *c);
   GameConnection *getControllingClient();

   U32 getObjectTypeMask() { return mObjectTypeMask; }

   F32 getUpdatePriority(NetObject *scopeObject, U32 updateMask, S32 updateSkips);

   virtual void render();

   virtual bool getCollisionPoly(Vector<Point> &polyPoints);
   virtual bool getCollisionCircle(U32 stateIndex, Point &point, float &radius);

   virtual void processServerMove(Move *theMove);
   virtual void processClientMove(Move *theMove, bool replay);
   virtual void processServer(U32 deltaT);
   virtual void processClient(U32 deltaT);

   virtual void writeControlState(BitStream *stream);
   virtual void readControlState(BitStream *stream);

   virtual Point getRenderPos();
   virtual Point getActualPos();
   virtual Point getRenderVel() { return Point(); }
   virtual Point getActualVel() { return Point(); }

   virtual void setActualPos(Point p);

   virtual bool collide(GameObject *hitObject) { return false; }

   virtual void damageObject(DamageInfo *damageInfo);

   bool onGhostAdd(GhostConnection *theConnection);
   void disableCollision() { mDisableCollisionCount++; }
   void enableCollision() { mDisableCollisionCount--; }
   void addToDatabase();
   void removeFromDatabase();
   bool isCollisionEnabled() { return mDisableCollisionCount == 0; }


   virtual void processArguments(S32 argc, const char**argv);
};

};

#endif
