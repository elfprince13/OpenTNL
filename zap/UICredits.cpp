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

#include "../tnl/tnlRandom.h"
#include "UICredits.h"
#include "glutInclude.h"
#include <stdio.h>

namespace Zap
{

CreditsUserInterface gCreditsUserInterface;

void CreditsUserInterface::onActivate()
{
   // construct the creditsfx objects here, they will
   // get properly deleted when the CreditsUI 
   // destructor is envoked

   // add credits scroller first and make it active
   CreditsScroller *scroller = new CreditsScroller;
   scroller->setActive(true);

   // add CreditsFX effects below here, dont activate:
   // 

   // choose randomly another CreditsFX effect
   // aside from CreditsScroller to activate
   if(fxList.size() > 1)
   {
      U32 rand = Random::readI(0, fxList.size() - 1);
      while(fxList[rand]->isActive())
      {
         U32 value = Platform::getRealMilliseconds();
         Random::addEntropy((U8 *) &value, sizeof(U32));

         rand = Random::readI(0, fxList.size() - 1);
      }
      fxList[rand]->setActive(true);
   }
}

void CreditsUserInterface::render()
{
   glViewport(0, 0, windowWidth, windowHeight);

   glClearColor(0, 0, 0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // loop through all the attached effects and 
   // call their render function
   for(S32 i = 0; i < fxList.size(); i++)
      if(fxList[i]->isActive())
         fxList[i]->render();
}

//-----------------------------------------------------
// CreditsFX Objects
//-----------------------------------------------------
CreditsFX::CreditsFX()
{
   activated = false;
   gCreditsUserInterface.addFX(this);
}

CreditsScroller::CreditsScroller()
{
   credits.clear();
   readCredits("credits.txt");
}

void CreditsScroller::updateFX(U32 delta)
{
   // scroll the credits text from bottom to top
   //  based on time
   for(S32 i = 0; i < credits.size(); i++)
   {
      S32 pos = credits[i].currPos.x - (delta / 10);

      // reached the top, reset
      if(pos < -CreditSpace)
         pos = gCreditsUserInterface.canvasHeight + CreditSpace * 3;
 
      credits[i].currPos.x = pos;
   }
}

void CreditsScroller::render()
{
   // draw the credits text
   glColor3f(1,1,1);
   for(S32 i = 0; i < credits.size(); i++)
   {
      gCreditsUserInterface.drawCenteredString(credits[i].currPos.x, 25, credits[i].jobBuf);
      gCreditsUserInterface.drawCenteredString(credits[i].currPos.x + 50, 25, credits[i].nameBuf);
   }
}

void CreditsScroller::readCredits(const char *file)
{
   // try and open the file context
   FILE *f = fopen(file, "r");
   if(!f)
      return;

   char name[MaxCreditLen];
   char job[MaxCreditLen];
   S32 pos = gCreditsUserInterface.canvasHeight + CreditSpace;
   
   // loop through each line in the credits file, expecting
   // the credits to be listed in this order:
   //    1) job
   //    2) name
   while(!feof(f))
   {
      CreditsInfo c;

      // get job
      fgets(job, MaxCreditLen, f);
      strcpy(c.jobBuf, job);

      // get name
      fgets(name, MaxCreditLen, f);
      strcpy(c.nameBuf, name);

      // place credit in cache
      c.currPos.x = pos;
      credits.push_back(c);

      pos += CreditSpace;
   }

   // close the file context
   fclose(f);
}

};