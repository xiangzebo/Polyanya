/*
 * $Id: sample.cpp,v 1.23 2006/11/01 23:33:56 nathanst Exp $
 *
 *  sample.cpp
 *  hog
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Copyright 2005 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <CircleGraph.h>

#define SG_RUNNING_IN_HOG

//#define ZOOM_TO_MAP

#include "Common.h"
#include "Sample.h"
#include "Map2DEnvironment.h"
#include "CPUTimer.h"
#include "Manager.h"

bool mouseTracking = false;
int px1, py1, px2, py2;
int mazeSize = 64;
int gStepsPerFrame = 4;

std::vector<ManagerBase*> managers;
int active_manager = 0;

/*
#ifdef USE_LATTICE
Manager<LatticeSG> manager;
#endif
#ifdef USE_GRID
Manager<Grid2DSGManager> manager;
#endif
*/

//#define CIRCLE_GRAPH
#ifdef CIRCLE_GRAPH
std::vector<CircleGraph> circle;
int circle_id = 0;
int circle_nodes = 20;
#endif

Map *map;
xyLoc start, goal;

unsigned int ssCount = 0;

MapEnvironment* mEnv;

int main(int argc, char* argv[]) {
  InstallHandlers();

  if (argc > 2 && strcmp(argv[1], "-map") == 0) {
    strncpy(gDefaultMap, argv[2], 1024);
    std::cout<<"HELELOY: "<<gDefaultMap<<std::endl;

    map = new Map(gDefaultMap);
    int x = map->GetMapWidth();
    int y = map->GetMapHeight();
    delete map;

    double r = (double)x/(double)y;
    double x_r = 1000;
    double y_r = 1000;

    if (r < 1) {
      x_r *= r;
    }
    else {
      y_r /= r;
    }


    RunHOGGUI(argc, argv, x_r, y_r);
  }
  else {
    RunHOGGUI(argc, argv, 500);
  }
}

//ContractionHierarchy *ch;
/**
 * This function is used to allocate the unit simulated that you want to run.
 * Any parameters or other experimental setup can be done at this time.
 */
void CreateSimulation(int id) {
  if (gDefaultMap[0] == 0) {
    srandom(10);
    map = new Map(mazeSize, mazeSize);
    //MakeMaze(map, 2);
    //map->Scale(mazeSize, mazeSize);
  } else {
    map = new Map(gDefaultMap);
//		map->Scale(512, 512);
  }
  map->SetTileSet(kWinter);
//  map->SetTileSet(kWinter);
  //map->SetTileSet(kWinterTile);
  mEnv = new MapEnvironment(map);


#ifdef USE_GRID
  managers.push_back(new Manager<Grid2DSG>());
#endif
#ifdef USE_LATTICE
  managers.push_back(new Manager<LatticeSG>());
#endif
  for (auto m : managers)
    m->Initialize(gDefaultMap);

//  resizeWindow(200,400);
#ifdef ZOOM_TO_MAP
  SetZoom(0, 6);
#endif

#ifdef CIRCLE_GRAPH
  map->SetTileSet(kBitmap);
  circle.push_back(CircleGraph(circle_nodes, kNoContraction, 4));
  circle.push_back(CircleGraph(circle_nodes, kRegularContraction, 4));
  circle.push_back(CircleGraph(circle_nodes, kRContraction, 4));
  circle.push_back(CircleGraph(circle_nodes, kHeavyRContraction, 4));
#endif
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */

void InstallHandlers() {
  InstallKeyboardHandler(MyDisplayHandler, "Screenshot", "Take a screenshot",
                         kAnyModifier, 's');

  InstallKeyboardHandler(MyDisplayHandler, "Change Start Orientation",
                         "Changes the start orientation.", kAnyModifier, 'z');
  InstallKeyboardHandler(MyDisplayHandler, "Change Goal Orientation",
                         "Changes the goal orientation.", kAnyModifier, 'x');

  InstallKeyboardHandler(MyDisplayHandler, "Random Problem",
                         "Generate a random problem.", kAnyModifier, 'r');
  InstallKeyboardHandler(MyDisplayHandler, "Solve Again",
                         "Solve the last problem again.", kAnyModifier, 'e');

  InstallKeyboardHandler(MyDisplayHandler, "Change exploration direction",
                         "Change exploration direction.", kAnyModifier, 'b');

  InstallKeyboardHandler(MyDisplayHandler, "Enter query",
                         "Enter two xylocs from the keyboard.", kAnyModifier,
                         'q');

  InstallKeyboardHandler(MyDisplayHandler, "Enter node or subgoal id",
                         "Enter node id or subgoal id (shift).", kAnyModifier,
                         'i');

  InstallKeyboardHandler(MyDisplayHandler, "H-Reachable Area Display",
                         "Change how the h-reachable area is displayed.",
                         kAnyModifier, 'a');

  InstallKeyboardHandler(MyDisplayHandler, "Search Tree Display",
                         "Change how the search tree is displayed.",
                         kAnyModifier, 't');

  InstallKeyboardHandler(MyDisplayHandler, "Path Display",
                         "Change how the path is displayed.", kAnyModifier,
                         'v');
  InstallKeyboardHandler(MyDisplayHandler, "Subgoal Graph Display",
                         "Change how the subgoal graph is displayed.",
                         kAnyModifier, 'g');

  InstallKeyboardHandler(MyDisplayHandler, "Construct Subgoal Graph",
                         "Construct the subgoal graph.", kAnyModifier, 'c');

  InstallKeyboardHandler(MyDisplayHandler, "CH", "CH",
                         kAnyModifier, 'l');

  InstallKeyboardHandler(MyDisplayHandler, "Circular graph", "Circular graph",
                         kAnyModifier, 'm');

  InstallCommandLineHandler(MyCLHandler, "-map", "-map filename",
                            "Selects the default map to be loaded.");
  InstallCommandLineHandler(MyCLHandler, "-convert", "-map file1 file2",
                            "Converts a map and saves as file2, then exits");
  InstallCommandLineHandler(
      MyCLHandler,
      "-size",
      "-batch integer",
      "If size is set, we create a square maze with the x and y dimensions specified.");

  InstallKeyboardHandler(MyDisplayHandler, "Choose method to display",
                         "Choose a method to display",
                         kAnyModifier, '0', '9');

  InstallWindowHandler(MyWindowHandler);

  InstallMouseClickHandler(MyClickHandler);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType) {
  if (eType == kWindowDestroyed) {
    printf("Window %ld destroyed\n", windowID);
    RemoveFrameHandler(MyFrameHandler, windowID, 0);
    mouseTracking = false;
    delete mEnv;
  } else if (eType == kWindowCreated) {
    printf("Window %ld created\n", windowID);
    InstallFrameHandler(MyFrameHandler, windowID, 0);
    CreateSimulation(windowID);
    SetNumPorts(windowID, 1);
  }
}

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *) {
  mEnv->OpenGLDraw();
  if (mouseTracking) {
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    Map *m = mEnv->GetMap();
    GLdouble x, y, z, r;
    m->GetOpenGLCoord(px1, py1, x, y, z, r);
    glVertex3f(x, y, z - 3 * r);
    m->GetOpenGLCoord(px2, py2, x, y, z, r);
    glVertex3f(x, y, z - 3 * r);
    glEnd();
  }
  managers[active_manager]->Visualize(mEnv);

#ifdef CIRCLE_GRAPH
  circle[circle_id].Draw(mEnv);
#endif
}

int MyCLHandler(char *argument[], int maxNumArgs) {
  if (strcmp(argument[0], "-map") == 0) {
    if (maxNumArgs <= 1)
      return 0;
    strncpy(gDefaultMap, argument[1], 1024);

    //doExport();
    return 2;
  } else if (strcmp(argument[0], "-convert") == 0) {
    if (maxNumArgs <= 2)
      return 0;
    Map m(argument[1]);
    m.Save(argument[2]);
    strncpy(gDefaultMap, argument[1], 1024);
    return 3;
  } else if (strcmp(argument[0], "-size") == 0) {
    if (maxNumArgs <= 1)
      return 0;
    mazeSize = atoi(argument[1]);
    assert(mazeSize > 0);
    return 2;
  }
  return 2;  //ignore typos
}

void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key) {
  switch (key) {
    case 's': {
      char filename[20];
      //double epsilon;
      //myFollower->GetEpsilon(epsilon);
      ssCount++;

      sprintf(filename, "%u", ssCount);

      //sprintf(filename, "%s", manager.GetScreenshotName());

      SaveScreenshot(windowID, filename);
      std::cout << "Screenshot saved! Filename: " << filename << std::endl;
    }
      break;

    case 'r':  // Random problem
    {
      int x1 = rand() % map->GetMapWidth();
      int y1 = rand() % map->GetMapHeight();

      while (!(map->GetTerrainType(x1, y1) == kGround)) {
        x1 = rand() % map->GetMapWidth();
        y1 = rand() % map->GetMapHeight();
      }

      int x2 = rand() % map->GetMapWidth();
      int y2 = rand() % map->GetMapHeight();

      while (!(map->GetTerrainType(x2, y2) == kGround)) {
        x2 = rand() % map->GetMapWidth();
        y2 = rand() % map->GetMapHeight();
      }

      start = xyLoc(x1, y1);
      goal = xyLoc(x2, y2);

      managers[active_manager]->DoublePointQuery(start, goal);
    }
      break;
    case 'e':  // Repeat previous problem
    {
      managers[active_manager]->DoublePointQuery(start,goal);
      //managers[active_manager]->SinglePointQuery(start);
    }
      break;

    case 'm':  // Repeat previous problem
    {
      if (mod == kShiftDown)
        active_manager = (active_manager + managers.size()-1) % managers.size();
      else
        active_manager = (active_manager + 1) % managers.size();

      std::cout << "Switched manager to: "
                << managers[active_manager]->GetName() << std::endl;
    }
      break;

#ifdef CIRCLE_GRAPH
    case 'm': {
      if (mod == kShiftDown)
        circle_id = (circle_id - 1 + circle.size()) % circle.size();
      else
        circle_id = (circle_id + 1) % circle.size();
      break;
    }
#endif

    // TODO: ctrl not working, using alt instead.
    default:
      managers[active_manager]->ProcessKeyboardCommand(key, mod == kShiftDown,
                                                       mod == kAltDown);
      break;
  }
}

void MyRandomUnitKeyHandler(unsigned long windowID, tKeyboardModifier, char) {

}

void MyPathfindingKeyHandler(unsigned long windowID, tKeyboardModifier, char) {

}

bool MyClickHandler(unsigned long windowID, int, int, point3d loc,
                    tButtonType button, tMouseEventType mType) {
  mouseTracking = false;
  if (button == kRightButton) {
    switch (mType) {
      case kMouseDown: {
        mEnv->GetMap()->GetPointFromCoordinate(
            loc, px1, py1);
        xyLoc query(px1, py1);
      }
        break;
      case kMouseDrag:
        mouseTracking = true;
        mEnv->GetMap()->GetPointFromCoordinate(
            loc, px2, py2);
        //printf("Mouse tracking at (%d, %d)\n", px2, py2);
        break;
      case kMouseUp: {
        if ((px1 == -1) || (px2 == -1))
          break;
        mEnv->GetMap()->GetPointFromCoordinate(
            loc, px2, py2);

        //Searching from (176, 179) to (286, 418)
        //px1 = 176;	py1 = 179;	px2 = 286;	py2 = 418;
        //Searching from (102, 376) to (298, 418)
        //px1 = 102;	py1 = 376;	px2 = 298;	py2 = 418;

        printf("Searching from (%d, %d) to (%d, %d)\n", px1, py1, px2, py2);

        start = xyLoc(px1, py1);
        goal = xyLoc(px2, py2);

        //(134, 107)
        //start = xyLoc(134, 107);
        //goal = xyLoc(134, 107);

        //sg->GetPath(start,goal);
        if (start == goal)
          managers[active_manager]->SinglePointQuery(start);
        else
          managers[active_manager]->DoublePointQuery(start, goal);

//				unitSims[windowID]->GetStats()->EnablePrintOutput(true);
//				SetNumPorts(windowID,2);
      }
        break;
    }
    return true;
  }
  return false;
}
