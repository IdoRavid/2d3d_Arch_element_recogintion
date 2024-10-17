#pragma once
#include "FloorRec.h"
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

class Wall {
public:
	API_Guid guid;
	API_Coord begC, endC;
	Wall::Wall(API_Guid INguid);

	Wall();


};



class FloorRec
{
	
public:
	static bool IsWallExternal(API_Guid guid);
	static int CreateRandInt(int low, int high);
	static void GetInternalWall();
	static bool ChecIfWallExternal(API_Guid guid);
	static GS::Array<API_Guid> GetExternalWalls();
	static void SelectExternalWalls();
	static API_Guid GetClosestPoint(std::map<API_Guid, Wall> map1, API_Coord point);
	static double GetElemArea(API_Guid guid);
	static API_Guid CreateFloorPolygon();
	static void CalculateFloorArea();
	static void ChangebegBool(bool val);
	static API_Guid CreateFillFromCoordArr(GS::Array<API_Coord> cArr, short pen);
	static GS::Array<API_Coord> DeleteDupCoords(GS::Array<API_Coord> cArr);
	static GS::Array <API_Coord> CreateFloorcArr();
	static void TryConnectWalls(API_Guid a, API_Guid b, double dist);
	static void AutoConnectWalls(double dist);
	static GS::Array<API_Guid> GetLinkedWalls(API_Guid FirstWall);
	static void CreateFloorElemFromContour();
	static void CreateFloorFromCoordArr(GS::Array<API_Coord> cArr);
	static std::pair<API_Guid, API_Guid> GetConnectedWallsPair(API_Guid wall);
	static GS::Array<API_Guid> GetConnectedWallsArray(API_Guid wall);
	static API_Guid GetWallsNearPoint(API_Coord coord, API_Guid ourGuid);
	static void IsSelectionExternalWall();
	static GS::Array<API_Guid> GetConnectingArray(API_Guid begWall, API_Guid endWall, bool IsBeg);
	static GS::Array<API_Guid> DeleteDupGuids(GS::Array<API_Guid> Arr);
	static API_Coord RayCast(Line ray, Line hit);
};

