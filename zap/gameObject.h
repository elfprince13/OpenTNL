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
#include "tnlNetObject.h"
#include "game.h"

namespace Zap
{

class GridDatabase;

enum GameObjectType
{
   UnknownType       = BIT(0),
   ShipType          = BIT(1),
   BarrierType       = BIT(2),
   MoveableType      = BIT(3),
   ProjectileType    = BIT(4),
   ItemType          = BIT(5),
   ResourceItemType  = BIT(6),
   EngineeredType    = BIT(7),
   ForceFieldType    = BIT(8),

   DeletedType       = BIT(30),
   CommandMapVisType = BIT(31),
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
   bool mInDatabase;

   Rect extent;
protected:
   U32 mObjectTypeMask;
   Move mLastMove; ///< the move for the previous update
   Move mCurrentMove; ///< The move for the current update
   S32 mTeam;
public:

   GameObject();
   ~GameObject() { removeFromGame(); }

   void addToGame(Game *theGame);
   virtual void onAddedToGame(Game *theGame);
   void removeFromGame();

   Game *getGame() { return mGame; }

   void deleteObject(U32 deleteTimeInterval = 0);
   bool isInDatabase() { return mInDatabase; }
   void setExtent(Rect &extentRect);
   Rect getExtent() { return extent; }
   S32 getTeam() { return mTeam; }
   void findObjects(U32 typeMask, Vector<GameObject *> &fillVector, Rect &extents);
   GameObject *findObjectLOS(U32 typeMask, U32 stateIndex, Point rayStart, Point rayEnd, float &collisionTime, Point &collisionNormal);

   bool isControlled() { return mControllingClient.isValid(); }
   void setControllingClient(GameConnection *c);
   GameConnection *getControllingClient();

   U32 getObjectTypeMask() { return mObjectTypeMask; }

   F32 getUpdatePriority(NetObject *scopeObject, U32 updateMask, S32 updateSkips);

   virtual S32 getRenderSortValue() { return 0; }
   virtual void render();

   /// render is called twice for every object that is in the
   /// render list.  By default GameObject will call the render()
   /// method one time (when layerIndex == 0).
   virtual void render(U32 layerIndex);

   virtual bool getCollisionPoly(Vector<Point> &polyPoints);
   virtual bool getCollisionCircle(U32 stateIndex, Point &point, float &radius);

   const Move &getCurrentMove() { return mCurrentMove; }
   const Move &getLastMove() { return mLastMove; }
   void setCurrentMove(const Move &theMove) { mCurrentMove = theMove; }
   void setLastMove(const Move &theMove) { mLastMove = theMove; }

   enum IdleCallPath {
      ServerIdleMainLoop,
      ServerIdleControlFromClient,
      ClientIdleMainRemote,
      ClientIdleControlMain,
      ClientIdleControlReplay,
   };

   virtual void idle(IdleCallPath path);

   virtual void writeControlState(BitStream *stream);
   virtual void readControlState(BitStream *stream);

   virtual void controlMoveReplayComplete();

   void writeCompressedVelocity(Point &vel, U32 max, BitStream *stream);
   void readCompressedVelocity(Point &vel, U32 max, BitStream *stream);

   virtual Point getRenderPos();
   virtual Point getActualPos();
   virtual Point getRenderVel() { return Point(); }
   virtual Point getActualVel() { return Point(); }

   virtual void setActualPos(Point p);

   virtual bool collide(GameObject *hitObject) { return false; }

   void radiusDamage(Point pos, F32 rad, U32 typemask, DamageInfo *info, F32 force = 2000.f);
   virtual void damageObject(DamageInfo *damageInfo);

   bool onGhostAdd(GhostConnection *theConnection);
   void disableCollision() { mDisableCollisionCount++; }
   void enableCollision() { mDisableCollisionCount--; }
   void addToDatabase();
   void removeFromDatabase();
   bool isCollisionEnabled() { return mDisableCollisionCount == 0; }

   virtual void processArguments(S32 argc, const char**argv);
};

extern void drawCircle(Point pos, F32 radius);

};

#endif
