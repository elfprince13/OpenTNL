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

#include "UIEditor.h"
#include "glutInclude.h"
#include "UICredits.h"
#include <stdio.h>

namespace Zap
{

bool EditorUserInterface::editorEnabled = false;

EditorUserInterface gEditorUserInterface;

void EditorUserInterface::setEditName(const char *name)
{
   mGameType[0] = 0;
   initLevelFromFile(name);
   if(mTeams.size() == 0)
   {
      Team t;
      t.color = Color(0, 0, 1);
      strcpy(t.name, "Blue");
      mTeams.push_back(t);
   }
   mCurrentOffset.set(0,0);
   mCurrentScale = 100;
   mCurrentPoly = -1;
   mCurrentVertex = -1;
   mCurrentItem = -1;
   mUp = mDown = mLeft = mRight = mIn = mOut = false;
   mCreatingPoly = false;
   strcpy(mEditFileName, name);
}

struct GameItemRec
{
   const char *name;
   bool hasTeam;
};

extern void renderFlag(Point pos, Color c);

enum GameItems
{
   ItemSpawn,
   ItemSoccerBall,
   ItemCTFFlag,
};

GameItemRec gGameItemRecs[] = {
   { "Spawn", true },
   { "SoccerBallItem", false },
   { "CTFFlagItem", true },
   { NULL, false },
};

void EditorUserInterface::processLevelLoadLine(int argc, const char **argv)
{
   U32 index;
   U32 strlenCmd = (U32) strlen(argv[0]);
   for(index = 0; gGameItemRecs[index].name != NULL; index++)
   {
      if(!strcmp(argv[0], gGameItemRecs[index].name) && 
         ( (argc == 3 && !gGameItemRecs[index].hasTeam) ||
           (argc == 4 && gGameItemRecs[index].hasTeam)))
           break;
   }

   if(gGameItemRecs[index].name)
   {
      Item i;
      i.index = index;
      if(gGameItemRecs[index].hasTeam)
      {
         i.team = atoi(argv[1]);
         i.pos.read(argv + 2);
      }
      else
      {
         i.team = -1;
         i.pos.read(argv + 1);
      }
      mItems.push_back(i);
   }
   else if(strlenCmd >= 8 && !strcmp(argv[0] + strlenCmd - 8, "GameType"))
   {
      strcpy(mGameType, argv[0]);
      mGameType[strlenCmd - 8] = 0;
      for(S32 i = 1; i < argc; i++)
         mGameTypeArgs.push_back(strdup(argv[i]));
   }
   else if(!strcmp(argv[0], "Team") && argc == 5)
   {
      Team t;
      strcpy(t.name, argv[1]);
      t.color.read(argv + 2);
      mTeams.push_back(t);
   }
   else if(!strcmp(argv[0], "BarrierMaker") && (argc & 1))
   {
      Poly poly;
      for(S32 i = 1; i < argc; i+= 2)
      {
         Point p(atof(argv[i]), atof(argv[i+1]));
         poly.verts.push_back(p);
      }
      if(poly.verts.size())
         mPolys.push_back(poly);
   }
   else
   {
      Vector<const char *> item;
      for(S32 i = 0; i < argc; i++)
         item.push_back(strdup(argv[i]));
      mUnknownItems.push_back(item);
   }
}

void EditorUserInterface::onActivate()
{
   editorEnabled = true;
}

Point EditorUserInterface::snapToLevelGrid(Point p)
{
   F32 mulFactor, divFactor;
   if(mCurrentScale >= 100)
   {
      mulFactor = 10;
      divFactor = 0.1;
   }
   else
   {
      mulFactor = 2;
      divFactor = 0.5;
   }

   return Point(floor(p.x * mulFactor + 0.5) * divFactor, 
                floor(p.y * mulFactor + 0.5) * divFactor);
}

void EditorUserInterface::render()
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glColor3f(0.0, 0.0, 0.0);
   glViewport(0, 0, U32(windowWidth), U32(windowHeight));

   glClearColor(0, 0, 0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if(mCurrentScale >= 100)
   {
      F32 gridScale = mCurrentScale * 0.1;
      F32 yStart = fmod(mCurrentOffset.y, gridScale);
      F32 xStart = fmod(mCurrentOffset.x, gridScale);
      glColor3f(0.2, 0.2, 0.2);
      glBegin(GL_LINES);
      while(yStart < canvasHeight)
      {
         glVertex2f(0, yStart);
         glVertex2f(canvasWidth, yStart);
         yStart += gridScale;
      }
      while(xStart < canvasWidth)
      {
         glVertex2f(xStart, 0);
         glVertex2f(xStart, canvasHeight);
         xStart += gridScale;
      }
      glEnd();
   }

   if(mCurrentScale >= 10)
   {
      F32 yStart = fmod(mCurrentOffset.y, mCurrentScale);
      F32 xStart = fmod(mCurrentOffset.x, mCurrentScale);
      glColor3f(0.4, 0.4, 0.4);
      glBegin(GL_LINES);
      while(yStart < canvasHeight)
      {
         glVertex2f(0, yStart);
         glVertex2f(canvasWidth, yStart);
         yStart += mCurrentScale;
      }
      while(xStart < canvasWidth)
      {
         glVertex2f(xStart, 0);
         glVertex2f(xStart, canvasHeight);
         xStart += mCurrentScale;
      }
      glEnd();
   }

   for(S32 i = 0; i < mItems.size(); i++)
   {
      renderItem(mItems[i]);
      if(mCurrentItem == i)
      {
         Point pos = convertLevelToCanvasCoord(mItems[i].pos);
         glColor3f(1,1,1);
         glBegin(GL_LINE_LOOP);
         glVertex2f(pos.x - 10, pos.y - 10);
         glVertex2f(pos.x + 10, pos.y - 10);
         glVertex2f(pos.x + 10, pos.y + 10);
         glVertex2f(pos.x - 10, pos.y + 10);
         glEnd();
      }
   }

   for(S32 i = 0; i < mPolys.size(); i++)
   {
      Poly &p = mPolys[i];
      if(mCurrentVertex == -1 && mCurrentPoly == i)
         glColor3f(1,1,0);
      else
         glColor3f(0,0,1);

      glLineWidth(3);
      renderPoly(p);
      glLineWidth(1);
      for(S32 j = 0; j < p.verts.size(); j++)
      {
         Point v = convertLevelToCanvasCoord(p.verts[j]);
         if(i == mCurrentPoly && j == mCurrentVertex)
            glColor3f(1,1,0);
         else
            glColor3f(1,0,0);
         glBegin(GL_LINE_LOOP);
         glVertex2f(v.x - 5, v.y - 5);
         glVertex2f(v.x + 5, v.y - 5);
         glVertex2f(v.x + 5, v.y + 5);
         glVertex2f(v.x - 5, v.y + 5);
         glEnd();
      }
   }
   if(mCreatingPoly)
   {
      Point mouseVertex = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      mNewPoly.verts.push_back(mouseVertex);
      glLineWidth(3);
      glColor3f(1, 1, 0);
      renderPoly(mNewPoly);
      glLineWidth(1);
      mNewPoly.verts.erase_fast(mNewPoly.verts.size() - 1);
   }
}

void EditorUserInterface::renderPoly(Poly &p)
{

   glBegin(GL_LINE_STRIP);
   for(S32 j = 0; j < p.verts.size(); j++)
   {
      Point v = convertLevelToCanvasCoord(p.verts[j]);
      glVertex2f(v.x, v.y);
   }
   glEnd();
}

void EditorUserInterface::renderItem(EditorUserInterface::Item &i)
{
   Point pos = convertLevelToCanvasCoord(i.pos);
   Color c;
   if(i.team == -1)
      c = Color(0.5, 0.5, 0.5);
   else
      c = mTeams[i.team].color;

   if(i.index == ItemCTFFlag)
   {
      glPushMatrix();
      glTranslatef(pos.x, pos.y, 0);
      glScalef(0.6, 0.6, 1);
      renderFlag(Point(0,0), c);
      glPopMatrix();
   }
   else
   {
      glColor3f(c.r, c.g, c.b);
      glBegin(GL_POLYGON);
      glVertex2f(pos.x - 8, pos.y - 8);
      glVertex2f(pos.x + 8, pos.y - 8);
      glVertex2f(pos.x + 8, pos.y + 8);
      glVertex2f(pos.x - 8, pos.y + 8);
      glEnd();
   }
}

void EditorUserInterface::findHitVertex(Point canvasPos)
{
   for(S32 i = mPolys.size() - 1; i >= 0; i--)
   {
      Poly &p = mPolys[i];
      for(S32 j = p.verts.size() - 1; j >= 0; j--)
      {
         Point v = convertLevelToCanvasCoord(p.verts[j]);
         if(fabs(v.x - canvasPos.x) < 5 && fabs(v.y - canvasPos.y) < 5)
         {
            mCurrentPoly = i;
            mCurrentVertex = j;
            return;
         }
      }
   }
}

void EditorUserInterface::findHitPoly(Point canvasPos)
{
   for(S32 i = mPolys.size() - 1; i >= 0; i--)
   {
      Poly &p = mPolys[i];
      Point p1 = convertLevelToCanvasCoord(p.verts[0]);
      for(S32 j = 0; j < p.verts.size() - 1; j++)
      {
         Point p2 = convertLevelToCanvasCoord(p.verts[j+1]);

         Point edgeDelta = p2 - p1;
         Point clickDelta = canvasPos - p1;
         float fraction = clickDelta.dot(edgeDelta);
         float lenSquared = edgeDelta.dot(edgeDelta);
         if(fraction > 0 && fraction < lenSquared)
         {
            // compute the closest point:
            Point closest = p1 + edgeDelta * (fraction / lenSquared);
            float distance = (canvasPos - closest).len();
            if(distance < 5)
            {
               mCurrentEdge = j;
               mCurrentPoly = i;
               mCurrentVertex = -1;
               return;
            }
         }
         p1 = p2;
      }
   }
}

void EditorUserInterface::findHitItem(Point canvasPos)
{
   for(S32 i = 0; i < mItems.size(); i++)
   {
      Item &itm = mItems[i];
      Point p = convertLevelToCanvasCoord(itm.pos);
      if(fabs(canvasPos.x - p.x) < 8 && fabs(canvasPos.y - p.y) < 8)
      {
         mCurrentItem = i;
         return;
      }
   }
}

void EditorUserInterface::onMouseDown(S32 x, S32 y)
{
   mMousePos = convertWindowToCanvasCoord(Point(x,y));
   if(mCreatingPoly)
   {
      if(mNewPoly.verts.size() > 1)
         mPolys.push_back(mNewPoly);
      mNewPoly.verts.clear();
      mCreatingPoly = false;
   }

   mMouseDownPos = mMousePos;
   mCurrentPoly = -1;
   mCurrentVertex = -1;
   mCurrentItem = -1;
   findHitVertex(mMousePos);
   if(mCurrentPoly == -1)
   {
      findHitPoly(mMousePos);
      if(mCurrentPoly != -1)
         mOriginalPoly = mPolys[mCurrentPoly];
      else
         findHitItem(mMousePos);
   }
}

void EditorUserInterface::onMouseUp(S32 x, S32 y)
{
}

void EditorUserInterface::onMouseDragged(S32 x, S32 y)
{
   mMousePos = convertWindowToCanvasCoord(Point(x,y));
   if(mCreatingPoly)
      return;

   if(mCurrentPoly != -1 && mCurrentVertex != -1)
   {
      Point newPos = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      mPolys[mCurrentPoly].verts[mCurrentVertex] = newPos;
   }
   else if(mCurrentPoly != -1)
   {
      Point delta = convertCanvasToLevelCoord(mMousePos) - convertCanvasToLevelCoord(mMouseDownPos);
      delta = snapToLevelGrid(delta);
      mPolys[mCurrentPoly] = mOriginalPoly;
      for(S32 i = 0; i < mPolys[mCurrentPoly].verts.size(); i++)
         mPolys[mCurrentPoly].verts[i] += delta;
   }
   else if(mCurrentItem != -1)
   {
      Point newPos = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      mItems[mCurrentItem].pos = newPos;
   }
}

void EditorUserInterface::onRightMouseDown(S32 x, S32 y)
{
   mMousePos = convertWindowToCanvasCoord(Point(x,y));
   if(mCreatingPoly)
   {
      Point newVertex = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      mNewPoly.verts.push_back(newVertex);
      return;
   }

   mCurrentPoly = -1;
   findHitPoly(mMousePos);
   if(mCurrentPoly != -1)
   {
      Point newVertex = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      // insert an extra vertex at the mouse clicked point,
      // and then select it.
      mPolys[mCurrentPoly].verts.insert(mCurrentEdge + 1);
      mPolys[mCurrentPoly].verts[mCurrentEdge + 1] = newVertex;
      mCurrentVertex = mCurrentEdge + 1;
   }
   else
   {
      //london chikara kelly markling
      mCreatingPoly = true;
      mNewPoly.verts.clear();
      Point newVertex = snapToLevelGrid(convertCanvasToLevelCoord(mMousePos));
      mNewPoly.verts.push_back(newVertex);
   }
}

void EditorUserInterface::onMouseMoved(S32 x, S32 y)
{
   mMousePos = convertWindowToCanvasCoord(Point(x,y));
}

void EditorUserInterface::deleteSelection()
{
   if(mCurrentPoly != -1 && mCurrentVertex != -1)
      mPolys[mCurrentPoly].verts.erase(mCurrentVertex);
   else if(mCurrentPoly != -1)
      mPolys.erase(mCurrentPoly);
   mCurrentPoly = mCurrentVertex = -1;
}

void EditorUserInterface::onKeyDown(U32 key)
{
   switch(tolower(key))
   {
      case 'r':
         mCurrentOffset.set(0,0);
         break;
      case 'w':
         mUp = true;
         break;
      case 's':
         mDown = true;
         break;
      case 'a':
         mLeft = true;
         break;
      case 'd':
         mRight = true;
         break;
      case 'e':
         mIn = true;
         break;
      case 'c':
         mOut = true;
         break;
      case 8:
      case 127:
         deleteSelection();
         break;
      case 27:
         gEditorMenuUserInterface.activate();
         break;
   }
}

void EditorUserInterface::onKeyUp(U32 key)
{
   switch(tolower(key))
   {
      case 'w':
         mUp = false;
         break;
      case 's':
         mDown = false;
         break;
      case 'a':
         mLeft = false;
         break;
      case 'd':
         mRight = false;
         break;
      case 'e':
         mIn = false;
         break;
      case 'c':
         mOut = false;
         break;
   }
}

void EditorUserInterface::idle(U32 timeDelta)
{
   glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
   F32 pixelsToScroll = timeDelta * 0.5f;

   if(mLeft && !mRight)
      mCurrentOffset.x += pixelsToScroll;
   else if(mRight && !mLeft)
      mCurrentOffset.x -= pixelsToScroll;
   if(mUp && !mDown)
      mCurrentOffset.y += pixelsToScroll;
   else if(mDown && !mUp)
      mCurrentOffset.y -= pixelsToScroll;

   Point mouseLevelPoint = convertCanvasToLevelCoord(mMousePos);
   if(mIn && !mOut)
      mCurrentScale *= 1 + timeDelta * 0.002;
   if(mOut && !mIn)
      mCurrentScale *= 1 - timeDelta * 0.002;
   if(mCurrentScale > 100)
      mCurrentScale = 100;
   else if(mCurrentScale < 10)
      mCurrentScale = 10;
   Point newMousePoint = convertLevelToCanvasCoord(mouseLevelPoint);
   mCurrentOffset += mMousePos - newMousePoint;   
}

void EditorUserInterface::saveLevel()
{
   FILE *f = fopen(mEditFileName, "w");
   fprintf(f, "%sGameType", mGameType);
   for(S32 i = 0; i < mGameTypeArgs.size(); i++)
      fprintf(f, " %s", mGameTypeArgs[i]);
   fprintf(f, "\n");
   for(S32 i = 0; i < mTeams.size(); i++)
   {
      fprintf(f, "Team %s %g %g %g\n", mTeams[i].name,
         mTeams[i].color.r, mTeams[i].color.g, mTeams[i].color.b);
   }
   for(S32 i = 0; i < mUnknownItems.size(); i++)
   {
      Vector<const char *> &v = mUnknownItems[i];
      for(S32 j = 0; j < v.size(); j++)
      {
         fputs(v[j], f);
         if(j == v.size() - 1)
            fputs("\n", f);
         else
            fputs(" ", f);
      }
   }
   for(S32 i = 0; i < mItems.size(); i++)
   {
      if(gGameItemRecs[mItems[i].index].hasTeam)
         fprintf(f, "%s %d %g %g\n", gGameItemRecs[mItems[i].index].name,
            mItems[i].team, mItems[i].pos.x, mItems[i].pos.y);
      else
         fprintf(f, "%s %g %g\n", gGameItemRecs[mItems[i].index].name,
            mItems[i].pos.x, mItems[i].pos.y);
   }
   for(S32 i = 0; i < mPolys.size(); i++)
   {
      Poly &p = mPolys[i];
      fputs("BarrierMaker ", f);
      for(S32 j = 0; j < p.verts.size(); j++)
         fprintf(f, "%g %g%c", p.verts[j].x, p.verts[j].y, (j == p.verts.size() - 1) ? '\n' : ' ');
   }

   fclose(f);
}

extern void hostGame(bool dedicated, Address bindAddress);
extern const char *gLevelList;

void EditorUserInterface::testLevel()
{
   char tmpFileName[256];
   strcpy(tmpFileName, mEditFileName);
   strcpy(mEditFileName, "editor.tmp");
   saveLevel();
   strcpy(mEditFileName, tmpFileName);
   const char *gLevelSave = gLevelList;
   gLevelList = "editor.tmp";
   hostGame(false, Address(IPProtocol, Address::Any, 28000));
   gLevelList = gLevelSave;
}

EditorMenuUserInterface gEditorMenuUserInterface;

EditorMenuUserInterface::EditorMenuUserInterface()
{
   menuTitle = "EDITOR MENU:";
   clearBackground = false;
}

void EditorMenuUserInterface::onActivate()
{
   setupMenus();
}

void EditorMenuUserInterface::setupMenus()
{
   menuItems.clear();
   menuItems.push_back("RETURN TO EDITOR");
   if(OptionsMenuUserInterface::fullscreen)
      menuItems.push_back("SET WINDOWED MODE");
   else
      menuItems.push_back("SET FULLSCREEN MODE");
   menuItems.push_back("TEST LEVEL");
   menuItems.push_back("SAVE LEVEL");
   menuItems.push_back("QUIT");
}

void EditorMenuUserInterface::processSelection(U32 index)
{
   switch(index)
   {
      case 0:
         gEditorUserInterface.activate();
         break;
      case 1:
         gOptionsMenuUserInterface.processSelection(1);
         setupMenus();
         break;
      case 2:
         gEditorUserInterface.testLevel();
         break;
      case 3:
         gEditorUserInterface.activate();
         gEditorUserInterface.saveLevel();
         break;
      case 4:
         gCreditsUserInterface.activate();
         break;
   }
}

void EditorMenuUserInterface::onEscape()
{
   gEditorUserInterface.activate();
}

void EditorMenuUserInterface::render()
{
   gEditorUserInterface.render();
   glColor4f(0, 0, 0, 0.5);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBegin(GL_POLYGON);
   glVertex2f(0, 0);
   glVertex2f(canvasWidth, 0);
   glVertex2f(canvasWidth, canvasHeight);
   glVertex2f(0, canvasHeight);
   glEnd();  
   glDisable(GL_BLEND); 
   glBlendFunc(GL_ONE, GL_ZERO);
   Parent::render();
}

};