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

#include "quickChat.h"
#include "glutInclude.h"
#include "gameType.h"

namespace Zap
{

VChatHelper::VChatNode VChatHelper::mChatTree[] =
{
   // Root node
   {0, ' ', true, "", ""},
      {1, 'V', true, "Offense", ""},
         {2, 'A', true, "Attack!",                 "Attack!"},
         {2, 'W', true, "Wait for signal",         "Wait for my signal to attack."},
         {2, 'C', true, "Cease fire",              "Cease fire!"},
         {2, 'M', true, "Move out",                "Move out."},
         {2, 'R', true, "Retreat",                 "Retreat!"},
         {2, 'H', true, "Hit deck",                "Hit the deck!"},
         {2, 'E', true, "Regroup",                 "Regroup."},
         {2, 'V', true, "Cover me",                "Cover me.."},
         {2, 'G', true, "Going offense",           "Going offense."},
         {2, 'O', true, "Go on the offensive",     "Go on the offensive."},
         {2, 'J', true, "Capture the objective",   "Capture the objective."},
      {1, 'T', true, "Target", ""},
         {2, 'A', true, "Target Acquired",            "Target Acquired."},
         {2, 'F', true, "Fire on my target",          "Fire on my target."},
         {2, 'N', true, "Target needed",              "I need a target."},
         {2, 'O', true, "Target out of range",        "Target is out of range."},
         {2, 'D', true, "Destroy Enemy Generator",    "Destroy the enemy generator."},
         {2, 'E', true, "Enemy Generator Destroyed",  "Enemy generator is destroyed."},
         {2, 'T', true, "Destroy Enemy Turret",       "Destroy enemy turret."},
         {2, 'S', true, "Enemy Turret Destroyed",     "Enemy turret destroyed."},
      {1, 'D', true, "Defense", ""},
         {2, 'I', true, "Incoming Enemies",           "Incoming enemies!"},
         {2, 'A', true, "Attacked",                   "We are being attacked."},
         {2, 'E', true, "Enemy Attacking Base",       "The enemy is attacking our base."},
         {2, 'N', true, "Need More Defense",          "We need more defense."},
         {2, 'T', true, "Base Taken",                 "Base is taken."},
         {2, 'C', true, "Base Clear",                 "Base is secured."},
         {2, 'Q', true, "Is Base Clear?",             "Is our base clear?"},
         {2, 'G', true, "Go On Defensive",            "Go on the defensive."},
         {2, 'D', true, "Defending Base",             "Defending our base."},
         {2, 'O', true, "Defend Our Base",            "Defend our base."},
      {1, 'F', true, "Flag", ""},
         {2, 'G', true, "Flag gone",          "Our flag is not in the base!"},
         {2, 'E', true, "Enemy has flag",     "The enemy has our flag!"},
         {2, 'H', true, "Have enemy flag",    "I have the enemy flag."},
         {2, 'S', true, "Flag secure",        "Our flag is secure."},
         {2, 'R', true, "Return our flag",    "Return our flag to base."},
         {2, 'F', true, "Get enemy flag",     "Get the enemy flag."},
         {2, 'M', true, "Flag mined",         "Our flag is mined."},
         {2, 'C', true, "Clear mines",        "Clear the mines from our flag."},
         {2, 'D', true, "Mines cleared",      "Mines have been cleared."},
         {2, 'N', true, "Mine flag",          "Mine the flag."},
     {1, 'R', true, "Need", ""},
         {2, 'R', true, "Need Repairs",      "Need repairs."},
         {2, 'A', true, "Need APC Pickup",   "I need an APC pickup."},
         {2, 'E', true, "Need Escort",       "I need an escort back to base."},
         {2, 'T', true, "Need Ammo",         "Can anyone bring me some ammo?"},
      {1, 'E', true, "Team", ""},
         {2, 'W', true, "Watch Shooting",    "Watch where your shooting!"},
         {2, 'D', true, "Dont know",         "I don't know."},
         {2, 'Y', true, "Yes",               "Yes."},
         {2, 'N', true, "No",                "No."},
         {2, 'T', true, "Thanks",            "Thanks."},
         {2, 'A', true, "No Problem",        "No Problem."},
         {2, 'S', true, "Sorry",             "Sorry."},
         {2, 'Z', true, "Doh!",              "Doh!"},
         {2, 'O', true, "Oops!",             "Oops!"},
         {2, 'F', true, "Shazbot!",          "Shazbot!"},
         {2, 'Q', true, "Damnit!",           "Damnit!"},
         {2, 'C', true, "Ah Crap!",          "Ah Crap!"},
         {2, 'E', true, "Duh!",              "Duh!"},
         {2, 'X', true, "You moron!",        "You moron!"},
         {2, 'R', true, "Ready",             "Ready."},
         {2, 'B', true, "Belay order",       "Belay order"},
         {2, ' ', true, "Help!",             "Help!"},
      {1, 'S', true, "Incoming Enemies - Direction", ""},
         {2, 'W', true, "Incoming North",    "*** INCOMING NORTH ***"},
         {2, 'A', true, "Incoming West",     "*** INCOMING WEST ***"},
         {2, 'S', true, "Incoming East",     "*** INCOMING EAST ***"},
         {2, 'Z', true, "Incoming South",    "*** INCOMING SOUTH ***"},
         {2, 'D', true, "Incoming HEAVIES",  "*** INCOMING HEAVIES ***"},
      {1, 'G', true, "Global", ""},
         {2, 'Z', false, "Doh",             "Doh!"},
         {2, 'O', false, "Oops",            "Oops!"},
         {2, 'F', false, "Shazbot",         "Shazbot!"},
         {2, 'Q', false, "Damnit",          "Damnit!"},
         {2, 'C', false, "Crap",            "Ah Crap!"},
         {2, 'E', false, "Duh",             "Duh."},
         {2, 'X', false, "You moron!",      "You moron!"},
         {2, 'N', false, "No",              "No."},
         {2, 'Y', false, "Yes",             "Yes."},
         {2, 'D', false, "I don't know",    "I don't know."},
         {2, 'T', false, "Thanks",          "Thanks."},
         {2, 'A', false, "No Problem",      "No Problemo."},
         {2, 'H', false, "Hi",              "Hi."},
         {2, 'B', false, "Bye",             "Bye!"},
         {2, 'S', false, "Sorry",           "Sorry."},
         {2, 'W', false, "Whoohoo",         "Woo-hoo!"},
         {2, 'V', false, "Hmmmm...",        "Hmmm..."},
         {2, 'U', false, "Yeah!",           "Yeah!"},
         {2, 'R', false, "Rawr",            "RAWR!"},
      {1, 'R', true, "Reponses", ""},
         {2, 'A', true, "Acknowledge",          "Acknowledged."},
         {2, 'Z', true, "Objective completed",  "Objective completed."},
         {2, 'I', true, "Unable to complete",   "Unable to complete objective."},
      {1, 'T', true, "Taunts", ""},
         {2, 'Y', false, "Yoohoo!",             "Yoohoo!"},
         {2, 'H', false, "How'd THAT feel?",    "How'd THAT feel?"},
         {2, 'I', false, "I've had worse..."    "I've had worse..."},
         {2, 'M', false, "Missed me!",          "Missed me!"},
         {2, 'D', false, "Dance!",              "Dance!"},
         {2, 'C', false, "Come get some!",      "Come get some!"},

   // Terminate
   {0, ' ', false, "", ""}
};

VChatHelper::VChatHelper()
{
   mCurNode = &mChatTree[0];
   mVisible = false;
}

void VChatHelper::render()
{
   if(mVisible)
   {
      VChatNode *walk = mCurNode;
      U32 matchLevel = walk->depth + 1;
      walk++;

      S32 curPos = 600 - 20;

      const fontSize = 15;

      // First get to the end...
      while(walk->depth >= matchLevel)
         walk++;

      // Then draw bottom up...
      glColor3f(0.3, 1.0, 0.3);
      while(walk != mCurNode)
      {
         if(walk->depth == matchLevel)
         {
            UserInterface::drawStringf(20, curPos, fontSize, "%c - %s", walk->trigger, walk->caption);
            curPos -= fontSize + 4;
         }

         walk--;
      }

      // All done!
   }
}

void VChatHelper::show()
{
   mCurNode = &mChatTree[0];
   mVisible = true;
}

bool VChatHelper::isActive()
{
   return mVisible;
}

void VChatHelper::processKey(U32 key)
{
   // Escape...
   if(key == 27)
   {
      mVisible = false;
      return;
   }

   // Try to find a match if we can...

   // work in upper case...
   key = toupper(key);

   // Set up walk...
   VChatNode *walk = mCurNode;
   U32 matchLevel = walk->depth+1;
   walk++;

   // Iterate over anything at our desired depth or lower...
   while(walk->depth >= matchLevel)
   {

      // If it has the same key and depth...
      if(toupper(walk->trigger) == key && walk->depth == matchLevel)
      {
         // select it...
         mCurNode = walk;

         // If we're at a leaf (ie, next child down is higher or equal to us), then issue the chat and call it good...
         walk++;
         if(mCurNode->depth >= walk->depth)
         {
            GameType *gt = gClientGame->getGameType();

            if(gt)
               gt->c2sSendChat(!mCurNode->teamOnly, mCurNode->msg);

            mVisible = false;

            return;
         }
      }

      walk++;
   }

}


};