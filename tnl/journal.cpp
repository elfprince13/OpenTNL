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

#include "tnlJournal.h"

namespace TNL
{

JournalEntryRecord *JournalEntryRecord::mList = NULL;

void Journal::callEntry(const char *funcName, MarshalledCall *theCall)
{
   BitStream unmarshallData(theCall->marshalledData.getBuffer(), theCall->marshalledData.getBytePosition());
   theCall->unmarshall(&unmarshallData);
   for(JournalEntryRecord *walk = JournalEntryRecord::mList; walk; walk = walk->mNext)
   {
      if(!strcmp(walk->mFunctionName, funcName))
      {
         MethodPointer p;
         walk->getFuncPtr(p);
         theCall->dispatch((void *) this, &p);
         return;
      }
   }
}

};