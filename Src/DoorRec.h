#pragma once

#include "APIEnvir.h"




// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include "GeoAnalysis.h"
#include <algorithm>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include <map>
#include "FloorRec.h"
#include <array>
#include "WallRec.h"

class Door {
public:
	Line DoorLine;
	API_Coord OpeningCoord;
	bool reflected;
	bool oSide;

	Door();
	Door(Line line, API_Coord coord);

	static std::pair<bool, bool> Door::ConfigDoorBools(Line line, API_Coord coord, API_Guid WallGuid);

};


class DoorRec
{
public:
	static void CreateDoors();
	static void CreateDoorsFromDrawingLines();

	static void CreateDoorsFromDrawingPolys();

	static GS::Array<Door> GetDoorLinesFromPoly();

	static GS::Array <Door> GetDoorLines();

	static void CreateWallInDoorLine(GS::Array<Door> dArr);

	static void CreateDoorElement(Door door);

	static double GetDoorAngleFromArc(API_Element arc);
	static  Door CheckIfDoor(API_Element arc, Line line);
	static bool IsConnectedToArc(GS::UIndex arcInd);
};

