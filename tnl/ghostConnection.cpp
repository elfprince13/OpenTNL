//-----------------------------------------------------------------------------------
//
//   Torque Network Library
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

#include "tnl.h"
#include "tnlGhostConnection.h"
#include "tnlBitStream.h"
#include "tnlNetBase.h"
#include "tnlNetObject.h"
#include "tnlNetInterface.h"

namespace TNL {

class GhostAlwaysObjectEvent : public NetEvent
{
   SafePtr<NetObject> object;
   U32 ghostIndex;
   bool validObject;
public:
   GhostAlwaysObjectEvent(NetObject *obj = NULL, U32 index = 0)
     : NetEvent(NetEvent::GuaranteedOrdered, NetEvent::DirAny)
   {
      if(obj)
      {
         object = obj;
         ghostIndex = index;
      }
   }
   ~GhostAlwaysObjectEvent()
   {
   }
   
   void pack(EventConnection *ps, BitStream *bstream)
   {
      bstream->writeInt(ghostIndex, GhostConnection::GhostIdBitSize);

      if(bstream->writeFlag(!object.isNull()))
      {
         GhostConnection *gc = static_cast<GhostConnection *>(ps);
         S32 classId = object->getClassId(ps->getNetClassGroup());
         bstream->writeClassId(classId, NetClassTypeObject, ps->getNetClassGroup());
         object->packUpdate(gc, 0xFFFFFFFF, bstream);
      }
   }
   void write(EventConnection *ps, BitStream *bstream)
   {
      bstream->writeInt(ghostIndex, GhostConnection::GhostIdBitSize);
      if(bstream->writeFlag(validObject))
      {
         GhostConnection *gc = static_cast<GhostConnection *>(ps);
         S32 classId = object->getClassId(ps->getNetClassGroup());
         bstream->writeClassId(classId, NetClassTypeObject, ps->getNetClassGroup());
         object->packUpdate(gc, 0xFFFFFFFF, bstream);
      }
   }
   void unpack(EventConnection *ps, BitStream *bstream)
   {
      ghostIndex = bstream->readInt(GhostConnection::GhostIdBitSize);

      if(bstream->readFlag())
      {
         GhostConnection *gc = static_cast<GhostConnection *>(ps);
         S32 classId = bstream->readClassId(NetClassTypeObject, ps->getNetClassGroup());
         if(classId == -1)
         {
            ps->setLastError("Invalid packet.");
            return;
         }
         object = (NetObject *) Object::create(ps->getNetClassGroup(), NetClassTypeObject, classId);
         if(!object)
         {
            ps->setLastError("Invalid packet.");
            return;
         }
         object->mOwningConnection = gc;
         object->mNetFlags = NetObject::IsGhost;
         object->unpackUpdate(gc, bstream);
         object->setInterface(ps->getInterface());
         validObject = true;
      }
      else
      {
         object = new NetObject;
         validObject = false;
      }
   }
   void process(EventConnection *ps)
   {
      GhostConnection *gc = static_cast<GhostConnection *>(ps);
      gc->setGhostAlwaysObject(object, ghostIndex);
      object = NULL;
   }
   TNL_DECLARE_CLASS(GhostAlwaysObjectEvent);
};

TNL_IMPLEMENT_NETEVENT(GhostAlwaysObjectEvent, NetClassGroupGameMask, 0);

GhostConnection::GhostConnection()
{
   // ghost management data:
   mScopeObject = NULL;
   mGhostingSequence = 0;
   mGhosting = false;
   mScoping = false;
   mGhostArray = NULL;
   mGhostRefs = NULL;
   mGhostLookupTable = NULL;
   mLocalGhosts = NULL;
}

GhostConnection::~GhostConnection()
{
   clearAllPacketNotifies();

   // delete any ghosts that may exist for this connection, but aren't added
   if(mGhostArray)
      clearGhostInfo();
   deleteLocalGhosts();
   delete[] mLocalGhosts;
   delete[] mGhostLookupTable;
   delete[] mGhostRefs;
   delete[] mGhostArray;
}

void GhostConnection::setGhostTo(bool ghostTo)
{
   if(mLocalGhosts) // if ghosting to this is already enabled, silently return
      return;

   if(ghostTo)
   {
      mLocalGhosts = new NetObject *[MaxGhostCount];
      for(S32 i = 0; i < MaxGhostCount; i++)
         mLocalGhosts[i] = NULL;
   }
}

void GhostConnection::setGhostFrom(bool ghostFrom)
{
   if(mGhostArray)
      return;

   if(ghostFrom)
   {
      mGhostFreeIndex = mGhostZeroUpdateIndex = 0;
      mGhostArray = new GhostInfo *[MaxGhostCount];
      mGhostRefs = new GhostInfo[MaxGhostCount];
      S32 i;
      for(i = 0; i < MaxGhostCount; i++)
      {
         mGhostRefs[i].obj = NULL;
         mGhostRefs[i].index = i;
         mGhostRefs[i].updateMask = 0;
      }
      mGhostLookupTable = new GhostInfo *[GhostLookupTableSize];
      for(i = 0; i < GhostLookupTableSize; i++)
         mGhostLookupTable[i] = 0;
   }
}

void GhostConnection::packetDropped(PacketNotify *pnotify)
{
   Parent::packetDropped(pnotify);
   GhostPacketNotify *notify = static_cast<GhostPacketNotify *>(pnotify);

   GhostRef *packRef = notify->ghostList;
   // loop through all the packRefs in the packet

   while(packRef)
   {
      GhostRef *temp = packRef->nextRef;

      U32 updateFlags = packRef->mask;

      // figure out which flags need to be updated on the object
      for(GhostRef *walk = packRef->updateChain; walk && updateFlags; walk = walk->updateChain)
         updateFlags &= ~walk->mask;
     
      // for any flags we haven't updated since this (dropped) packet
      // or them into the mask so they'll get updated soon

      if(updateFlags)
      {
         if(!packRef->ghost->updateMask)
         {
            packRef->ghost->updateMask = updateFlags;
            ghostPushNonZero(packRef->ghost);
         }
         else
            packRef->ghost->updateMask |= updateFlags;
      }

      // make sure this packRef isn't the last one on the GhostInfo
      if(packRef->ghost->lastUpdateChain == packRef)
         packRef->ghost->lastUpdateChain = NULL;
      
      // if this packet was ghosting an object, set it
      // to re ghost at it's earliest convenience

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
      {
         packRef->ghost->flags |= GhostInfo::NotYetGhosted;
         packRef->ghost->flags &= ~GhostInfo::Ghosting;
      }
      
      // otherwise, if it was being deleted,
      // set it to re-delete

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
      {
         packRef->ghost->flags |= GhostInfo::KillGhost;
         packRef->ghost->flags &= ~GhostInfo::KillingGhost;
      }

      delete packRef;
      packRef = temp;
   }
}

void GhostConnection::packetReceived(PacketNotify *pnotify)
{
   Parent::packetReceived(pnotify);
   GhostPacketNotify *notify = static_cast<GhostPacketNotify *>(pnotify);

   GhostRef *packRef = notify->ghostList;

   // loop through all the notifies in this packet

   while(packRef)
   {
      // make sure this packRef isn't the last one on the GhostInfo
      if(packRef->ghost->lastUpdateChain == packRef)
         packRef->ghost->lastUpdateChain = NULL;

      GhostRef *temp = packRef->nextRef;      
      // if this object was ghosting , it is now ghosted

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
      {
         packRef->ghost->flags &= ~GhostInfo::Ghosting;
         if(packRef->ghost->obj)
            packRef->ghost->obj->onGhostAvailable(this);
      }
      // otherwise, if it was dieing, free the ghost

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
         freeGhostInfo(packRef->ghost);

      delete packRef;
      packRef = temp;
   }
}

struct UpdateQueueEntry
{
   F32 priority;
   GhostInfo *obj;

   UpdateQueueEntry(F32 in_priority, GhostInfo *in_obj) 
      { priority = in_priority; obj = in_obj; }
};

static S32 QSORT_CALLBACK UQECompare(const void *a,const void *b)
{
   GhostInfo *ga = *((GhostInfo **) a);
   GhostInfo *gb = *((GhostInfo **) b);

   F32 ret = ga->priority - gb->priority;
   return (ret < 0) ? -1 : ((ret > 0) ? 1 : 0);
} 

void GhostConnection::prepareWritePacket()
{
   Parent::prepareWritePacket();

   if(!doesGhostFrom() && !mGhosting)
      return;
   // first step is to check all our polled ghosts:

   // 1. Scope query - find if any new objects have come into
   //    scope and if any have gone out.

   // Each packet we loop through all the objects with non-zero masks and
   // mark them as "out of scope" before the scope query runs.
   // if the object has a zero update mask, we wait to remove it until it requests
   // an update

   for(S32 i = 0; i < mGhostZeroUpdateIndex; i++)
   {
      // increment the updateSkip for everyone... it's all good
      GhostInfo *walk = mGhostArray[i];
      walk->updateSkipCount++;
      if(!(walk->flags & (GhostInfo::ScopeAlways | GhostInfo::ScopeLocalAlways)))
         walk->flags &= ~GhostInfo::InScope;
   }

   if(mScopeObject)
      mScopeObject->performScopeQuery(this);
}

bool GhostConnection::isDataToTransmit()
{
   // once we've run the scope query - if there are no objects that need to be updated,
   // we return false
   return Parent::isDataToTransmit() || mGhostZeroUpdateIndex != 0;
}

void GhostConnection::writePacket(BitStream *bstream, PacketNotify *pnotify)
{
   Parent::writePacket(bstream, pnotify);
   GhostPacketNotify *notify = static_cast<GhostPacketNotify *>(pnotify);

   if(mConnectionParameters.mDebugObjectSizes)
      bstream->writeInt(DebugChecksum, 32);

   notify->ghostList = NULL;
   
   if(!doesGhostFrom())
      return;
   
   if(!bstream->writeFlag(mGhosting))
      return;
      
   // fill a packet (or two) with ghosting data

   // 2. call scoped objects' priority functions if the flag set is nonzero
   //    A removed ghost is assumed to have a high priority
   // 3. call updates based on sorted priority until the packet is
   //    full.  set flags to zero for all updated objects

   GhostInfo *walk;

   for(S32 i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
   {
      if(!(mGhostArray[i]->flags & GhostInfo::InScope))
         detachObject(mGhostArray[i]);
   }

   U32 maxIndex = 0;
   for(S32 i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
   {
      walk = mGhostArray[i];
      if(walk->index > maxIndex)
         maxIndex = walk->index;

      // clear out any kill objects that haven't been ghosted yet
      if((walk->flags & GhostInfo::KillGhost) && (walk->flags & GhostInfo::NotYetGhosted))
      {
         freeGhostInfo(walk);
         continue;
      }
      // don't do any ghost processing on objects that are being killed
      // or in the process of ghosting
      else if(!(walk->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting)))
      {
         if(walk->flags & GhostInfo::KillGhost)
            walk->priority = 10000;
         else
            walk->priority = walk->obj->getUpdatePriority(mScopeObject, walk->updateMask, walk->updateSkipCount);
      }
      else
         walk->priority = 0;
   }
   GhostRef *updateList = NULL;
   qsort(mGhostArray, mGhostZeroUpdateIndex, sizeof(GhostInfo *), UQECompare);
   // reset the array indices...
   for(S32 i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
      mGhostArray[i]->arrayIndex = i;

   S32 sendSize = 1;
   while(maxIndex >>= 1)
      sendSize++;

   if(sendSize < 3)
      sendSize = 3;

   bstream->writeInt(sendSize - 3, 3); // 0-7 3 bit number

   U32 count = 0;
   // 
   for(S32 i = mGhostZeroUpdateIndex - 1; i >= 0 && !bstream->isFull(); i--)
   {
      GhostInfo *walk = mGhostArray[i];
		if(walk->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting))
		   continue;
		   
      //S32 startPos = bstream->getCurPos();
      bstream->writeFlag(true);

      bstream->writeInt(walk->index, sendSize);
      U32 updateMask = walk->updateMask;
      
      GhostRef *upd = new GhostRef;

      upd->nextRef = updateList;
      updateList = upd;

      if(walk->lastUpdateChain)
         walk->lastUpdateChain->updateChain = upd;
      walk->lastUpdateChain = upd;

      upd->ghost = walk;
      upd->ghostInfoFlags = 0;
      upd->updateChain = NULL;

      if(walk->flags & GhostInfo::KillGhost)
      {
         walk->flags &= ~GhostInfo::KillGhost;
         walk->flags |= GhostInfo::KillingGhost;
         walk->updateMask = 0;
         upd->mask = updateMask;
         ghostPushToZero(walk);
         upd->ghostInfoFlags = GhostInfo::KillingGhost;
         bstream->writeFlag(true); // killing ghost
      }
      else 
      {
         bstream->writeFlag(false);
         S32 startPos = bstream->getBitPosition();

         if(mConnectionParameters.mDebugObjectSizes)
            bstream->advanceBitPosition(BitStreamPosBitSize);

         if(walk->flags & GhostInfo::NotYetGhosted)
         {
            S32 classId = walk->obj->getClassId(getNetClassGroup());
            bstream->writeClassId(classId, NetClassTypeObject, getNetClassGroup());

            walk->flags &= ~GhostInfo::NotYetGhosted;
            walk->flags |= GhostInfo::Ghosting;
            upd->ghostInfoFlags = GhostInfo::Ghosting;
         }

         // update the object
         U32 retMask = walk->obj->packUpdate(this, updateMask, bstream);

         if(mConnectionParameters.mDebugObjectSizes)
            bstream->writeIntAt(bstream->getBitPosition(), BitStreamPosBitSize, startPos);

         TNLLogMessageV(LogGhostConnection, ("GhostConnection %s GHOST %d", walk->obj->getClassName(), bstream->getBitPosition() - 16 - startPos));

         TNLAssert((retMask & (~updateMask)) == 0, "Cannot set new bits in packUpdate return");

         walk->updateMask = retMask;
         if(!retMask)
            ghostPushToZero(walk);

         upd->mask = updateMask & ~retMask;

         //PacketStream::getStats()->addBits(PacketStats::Send, bstream->getCurPos() - startPos, walk->obj->getPersistTag());
      }
      walk->updateSkipCount = 0;
      count++;
   }
   // count # of ghosts have been updated,
   // mGhostZeroUpdateIndex # of ghosts remain to be updated.
   // no more objects...
   bstream->writeFlag(false);
   notify->ghostList = updateList;
}

void GhostConnection::readPacket(BitStream *bstream)
{
   Parent::readPacket(bstream);

   if(mConnectionParameters.mDebugObjectSizes)
   {
      U32 sum = bstream->readInt(32);
      TNLAssert(sum == DebugChecksum, "Invalid checksum.");
   }

   if(!doesGhostTo())
      return;
   if(!bstream->readFlag())
      return;

   S32 idSize;
   idSize = bstream->readInt( 3 );
   idSize += 3;

   // while there's an object waiting...

   while(bstream->readFlag())
   {
      U32 index;
      //S32 startPos = bstream->getCurPos();
      index = (U32) bstream->readInt(idSize);
      if(bstream->readFlag()) // is this ghost being deleted?
      {
         TNLAssert(mLocalGhosts[index] != NULL, "Error, NULL ghost encountered.");
		   if(mLocalGhosts[index])
         {
            mLocalGhosts[index]->onGhostRemove();
		      delete mLocalGhosts[index];
            mLocalGhosts[index] = NULL;
         }
      }
      else
      {
         U32 endPosition = 0;
         if(mConnectionParameters.mDebugObjectSizes)
            endPosition = bstream->readInt(BitStreamPosBitSize);

         if(!mLocalGhosts[index]) // it's a new ghost... cool
         {
            S32 classId = bstream->readClassId(NetClassTypeObject, getNetClassGroup());
            if(classId == -1)
            {
               setLastError("Invalid packet.");
               return;
            }

            NetObject *obj = (NetObject *) Object::create(getNetClassGroup(), NetClassTypeObject, classId);
            if(!obj)
            {
               setLastError("Invalid packet.");
               return;
            }
            obj->mOwningConnection = this;
            obj->mNetFlags = NetObject::IsGhost;

            // object gets initial update before adding to the manager

            obj->mNetIndex = index;
            mLocalGhosts[index] = obj;
            mLocalGhosts[index]->setInterface(getInterface());
            mLocalGhosts[index]->unpackUpdate(this, bstream);
            
            if(!obj->onGhostAdd(this))
            {
               if(!mErrorBuffer[0])
                  setLastError("Invalid packet.");
               return;
            }
            if(mRemoteConnection)
            {
               GhostConnection *gc = static_cast<GhostConnection *>(mRemoteConnection.getPointer());
               obj->mServerObject = gc->resolveGhostParent(index);
            }
         }
         else
         {
            mLocalGhosts[index]->unpackUpdate(this, bstream);
         }

         if(mConnectionParameters.mDebugObjectSizes)
         {
            TNLAssert(bstream->getBitPosition() == endPosition,
            avar("unpackUpdate did not match packUpdate for object of class %s. Expected %d bits, got %d bits.",
               mLocalGhosts[index]->getClassName(), endPosition, bstream->getBitPosition()) );
         }

         if(mErrorBuffer[0])
            return;
      }
   }
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

void GhostConnection::setScopeObject(NetObject *obj)
{
   if(((NetObject *) mScopeObject) == obj)
      return;
   mScopeObject = obj;
}

void GhostConnection::detachObject(GhostInfo *info)
{
   // mark it for ghost killin'
   info->flags |= GhostInfo::KillGhost;

   // if the mask is in the zero range, we've got to move it up...
   if(!info->updateMask)
   {
      info->updateMask = 0xFFFFFFFF;
      ghostPushNonZero(info);
   }
   if(info->obj)
   {
      if(info->prevObjectRef)
         info->prevObjectRef->nextObjectRef = info->nextObjectRef;
      else
         info->obj->mFirstObjectRef = info->nextObjectRef;
      if(info->nextObjectRef)
         info->nextObjectRef->prevObjectRef = info->prevObjectRef;
      // remove it from the lookup table
      
      U32 id = info->obj->getHashId();
      for(GhostInfo **walk = &mGhostLookupTable[id & GhostLookupTableMask]; *walk; walk = &((*walk)->nextLookupInfo))
      {
         GhostInfo *temp = *walk;
         if(temp == info)
         {
            *walk = temp->nextLookupInfo;
            break;
         }
      }
      info->prevObjectRef = info->nextObjectRef = NULL;
      info->obj = NULL;
   }
}

void GhostConnection::freeGhostInfo(GhostInfo *ghost)
{
   TNLAssert(ghost->arrayIndex < mGhostFreeIndex, "Ghost already freed.");
   if(ghost->arrayIndex < mGhostZeroUpdateIndex)
   {
      TNLAssert(ghost->updateMask != 0, "Invalid ghost mask.");
      ghost->updateMask = 0;
      ghostPushToZero(ghost);
   }
   ghostPushZeroToFree(ghost);
   TNLAssert(ghost->lastUpdateChain == NULL, "Ack!");
}

//-----------------------------------------------------------------------------

void GhostConnection::objectLocalScopeAlways(NetObject *obj)
{
   if(!doesGhostFrom())
      return;
   objectInScope(obj);
   for(GhostInfo *walk = mGhostLookupTable[obj->getHashId() & GhostLookupTableMask]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags |= GhostInfo::ScopeLocalAlways;
      return;
   }
}

void GhostConnection::objectLocalClearAlways(NetObject *obj)
{
   if(!doesGhostFrom())
      return;
   for(GhostInfo *walk = mGhostLookupTable[obj->getHashId() & GhostLookupTableMask]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags &= ~GhostInfo::ScopeLocalAlways;
      return;
   }
}

bool GhostConnection::validateGhostArray()
{
   TNLAssert(mGhostZeroUpdateIndex >= 0 && mGhostZeroUpdateIndex <= mGhostFreeIndex, "Invalid update index range.");
   TNLAssert(mGhostFreeIndex <= MaxGhostCount, "Invalid free index range.");
   S32 i;
   for(i = 0; i < mGhostZeroUpdateIndex; i ++)
   {
      TNLAssert(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
      TNLAssert(mGhostArray[i]->updateMask != 0, "Invalid ghost mask.");
   }
   for(; i < mGhostFreeIndex; i ++)
   {
      TNLAssert(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
      TNLAssert(mGhostArray[i]->updateMask == 0, "Invalid ghost mask.");
   }
   for(; i < MaxGhostCount; i++)
   {
      TNLAssert(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
   }
   return true;
}

void GhostConnection::objectInScope(NetObject *obj)
{
   if (!mScoping || !doesGhostFrom())
      return;
	if (!obj->isGhostable() || (obj->isScopeLocal() && !isLocalConnection()))
		return;
   S32 index = obj->getHashId() & GhostLookupTableMask;
   
   // check if it's already in scope
   // the object may have been cleared out without the lookupTable being cleared
   // so validate that the object pointers are the same.

   for(GhostInfo *walk = mGhostLookupTable[index ]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags |= GhostInfo::InScope;
      return;
   }

   if (mGhostFreeIndex == MaxGhostCount)
      return;

   GhostInfo *giptr = mGhostArray[mGhostFreeIndex];
   ghostPushFreeToZero(giptr);
   giptr->updateMask = 0xFFFFFFFF;
   ghostPushNonZero(giptr);

   giptr->flags = GhostInfo::NotYetGhosted | GhostInfo::InScope;
   
   if(obj->mNetFlags.test(NetObject::ScopeAlways))
      giptr->flags |= GhostInfo::ScopeAlways;

   giptr->obj = obj;
   giptr->lastUpdateChain = NULL;
   giptr->updateSkipCount = 0;

   giptr->connection = this;

   giptr->nextObjectRef = obj->mFirstObjectRef;
   if(obj->mFirstObjectRef)
      obj->mFirstObjectRef->prevObjectRef = giptr;
   giptr->prevObjectRef = NULL;
   obj->mFirstObjectRef = giptr;
   
   giptr->nextLookupInfo = mGhostLookupTable[index];
   mGhostLookupTable[index] = giptr;
   //TNLAssert(validateGhostArray(), "Invalid ghost array!");
}

//-----------------------------------------------------------------------------

TNL_DECLARE_RPC_MEM_ENUM(GhostConnection, GhostCountBitSize);
TNL_IMPLEMENT_RPC(GhostConnection, rpcGhostAlwaysStarting, (U32 sequence, Int<GhostConnection::GhostCountBitSize> ghostCount), 
      NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirAny, 0)
{
   TNLLogMessageV(LogGhostConnection, ("Got GhostAlwaysStarting %d", sequence));

   if(!doesGhostTo())
   {
      setLastError("Invalid packet.");
      return;
   }
   onStartGhosting(ghostCount);
}

TNL_IMPLEMENT_RPC(GhostConnection, rpcGhostAlwaysDone, (U32 sequence), 
      NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirAny, 0)
{
   TNLLogMessageV(LogGhostConnection, ("Got GhostAlwaysDone %d", sequence));
   if(!doesGhostTo())
   {
      setLastError("Invalid packet.");
      return;
   }
   mGhostingSequence = sequence;
   rpcReadyForNormalGhosts(mGhostingSequence);   
}

TNL_IMPLEMENT_RPC(GhostConnection, rpcReadyForNormalGhosts, (U32 sequence), 
      NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirAny, 0)
{
   TNLLogMessageV(LogGhostConnection, ("Got ready for normal ghosts %d %d", sequence, mGhostingSequence));
   if(!doesGhostFrom())
   {
      setLastError("Invalid packet.");
      return;
   }
   if(sequence != mGhostingSequence)
      return;
   onAllGhostAlwaysObjectsReceived();
   mGhosting = true;
   for(S32 i = 0; i < mGhostFreeIndex; i++)
   {
      if(mGhostArray[i]->flags & GhostInfo::ScopedEvent)
      {
         mGhostArray[i]->flags &= ~(GhostInfo::Ghosting | GhostInfo::ScopedEvent);
         if(mGhostArray[i]->obj)
            mGhostArray[i]->obj->onGhostAvailable(this);
      }
   }
}

TNL_IMPLEMENT_RPC(GhostConnection, rpcEndGhosting, (), 
      NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirAny, 0)
{
   if(!doesGhostTo())
   {
      setLastError("Invalid packet.");
      return;
   }
   deleteLocalGhosts();
   onEndGhosting();
}

void GhostConnection::deleteLocalGhosts()
{
   if(!mLocalGhosts)
      return;
   // just delete all the local ghosts,
   // and delete all the ghosts in the current save list
   for(S32 i = 0; i < MaxGhostCount; i++)
   {
      if(mLocalGhosts[i])
      {
         mLocalGhosts[i]->onGhostRemove();
         delete mLocalGhosts[i];
         mLocalGhosts[i] = NULL;
      }
   }
}

void GhostConnection::activateGhosting()
{
   if(!doesGhostFrom())
      return;

   mGhostingSequence++;
   
   TNLAssert((mGhostFreeIndex == 0) && (mGhostZeroUpdateIndex == 0), "Error: ghosts in the ghost list before activate.");
   
   // iterate through the ghost always objects and InScope them...
   // also post em all to the other side.

   Vector<NetObject *> &ghostAlwaysSet = mInterface->getScopeAlwaysList();

   S32 sz = ghostAlwaysSet.size();
   S32 j;

   for(j = 0; j < sz; j++)
   {
      U32 idx = MaxGhostCount - sz + j;
      mGhostArray[j] = mGhostRefs + idx;
      mGhostArray[j]->arrayIndex = j;
   }
   for(j = sz; j < MaxGhostCount; j++)
   {
      U32 idx = j - sz;
      mGhostArray[j] = mGhostRefs + idx;
      mGhostArray[j]->arrayIndex = j;
   }
   mScoping = true; // so that objectInScope will work
   for(j = 0; j < sz; j++)
   {
      objectInScope(ghostAlwaysSet[j]);
   }
   rpcGhostAlwaysStarting(mGhostingSequence, ghostAlwaysSet.size());
   for(j = mGhostZeroUpdateIndex - 1; j >= 0; j--)
   {
      TNLAssert((mGhostArray[j]->flags & GhostInfo::ScopeAlways) != 0, "Non-scope always in the scope always list.")

      // we may end up resending state here, but at least initial state
      // will not be resent.
      mGhostArray[j]->updateMask = 0;
      ghostPushToZero(mGhostArray[j]);
      mGhostArray[j]->flags &= ~GhostInfo::NotYetGhosted;
      mGhostArray[j]->flags |= GhostInfo::ScopedEvent;

      postNetEvent(new GhostAlwaysObjectEvent(mGhostArray[j]->obj, mGhostArray[j]->index));
   }
   rpcGhostAlwaysDone(mGhostingSequence);
   //TNLAssert(validateGhostArray(), "Invalid ghost array!");
}

void GhostConnection::clearGhostInfo()
{
   // gotta clear out the ghosts...
   for(PacketNotify *walk = mNotifyQueueHead; walk; walk = walk->nextPacket)
   {
      GhostPacketNotify *note = static_cast<GhostPacketNotify *>(walk);
      GhostRef *delWalk = note->ghostList;
      note->ghostList = NULL;
      while(delWalk)
      {
         GhostRef *next = delWalk->nextRef;
         delete delWalk;
         delWalk = next;
      }
   }
   for(S32 i = 0; i < MaxGhostCount; i++)
   {
      if(mGhostRefs[i].arrayIndex < mGhostFreeIndex)
      {
         detachObject(&mGhostRefs[i]);
         freeGhostInfo(&mGhostRefs[i]);
      }
   }
   TNLAssert((mGhostFreeIndex == 0) && (mGhostZeroUpdateIndex == 0), "Invalid indices.");
}

void GhostConnection::resetGhosting()
{
   if(!doesGhostFrom())
      return;
   // stop all ghosting activity
   // send a message to the other side notifying of this
   
   mGhosting = false;
   mScoping = false;
   rpcEndGhosting();
   mGhostingSequence++;
   clearGhostInfo();
   //TNLAssert(validateGhostArray(), "Invalid ghost array!");
}

void GhostConnection::setGhostAlwaysObject(NetObject *object, U32 index)
{
   if(!doesGhostTo())
   {
	  delete object;
      setLastError("Invalid packet.");
      return;
   }
   object->mNetFlags = NetObject::IsGhost;
   // while there's an object waiting...
   if (isLocalConnection()) {
      GhostConnection *gc = static_cast<GhostConnection *>(mRemoteConnection.getPointer());
      object->mServerObject = gc->resolveGhostParent(index);
   }
   if(!object->onGhostAdd(this))
   {
      // make sure there's an error message if necessary
      if(!mErrorBuffer[0])
         setLastError("Invalid packet.");
      delete object;
      return;
   }
   TNLAssert(mLocalGhosts[index] == NULL, "Ghost already in table!");
   mLocalGhosts[index] = object;
   object->mNetIndex = index;
   onGhostAlwaysObjectReceived(object);
}

//-----------------------------------------------------------------------------

NetObject *GhostConnection::resolveGhost(S32 id)
{
   if(id == -1)
      return NULL;

   return mLocalGhosts[id];
}

NetObject *GhostConnection::resolveGhostParent(S32 id)
{
   return mGhostRefs[id].obj;
}

S32 GhostConnection::getGhostIndex(NetObject *obj)
{
   if(!obj)
      return -1;
   if(!doesGhostFrom())
      return obj->mNetIndex;
   S32 index = obj->getHashId() & GhostLookupTableMask;

   for(GhostInfo *gptr = mGhostLookupTable[index]; gptr; gptr = gptr->nextLookupInfo)
   {
      if(gptr->obj == obj && (gptr->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting | GhostInfo::NotYetGhosted | GhostInfo::KillGhost)) == 0)
         return gptr->index;
   }
   return -1;
}

//-----------------------------------------------------------------------------

void GhostConnection::onAllGhostAlwaysObjectsReceived()
{
}

void GhostConnection::onEndGhosting()
{
}

void GhostConnection::onStartGhosting(U32)
{
}

void GhostConnection::onGhostAlwaysObjectReceived(NetObject *)
{
}

};
