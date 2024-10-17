#pragma once

#include "APIEnvir.h"
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
#include "WindowRec.h"
#include "TmrTech.h"
class WallMatch
{
public:
	static GS::Array<std::pair<API_Guid, bool>> GetSingleWallsArr();


	static std::pair<bool, bool> IsWallSingle(API_Guid wall);

	static void ManageWallMatching();

	static void MatchSingleWalls();

	static void ConnectMatchingSinglesPerpendicular(GS::Array<std::pair<API_Guid, bool>> SingleWalls);

	static void ConnectMatchingSinglesParallel(GS::Array<std::pair<API_Guid, bool>> SingleWalls);

	static double GetWallWidth(API_Guid wall);

	static bool IsPointInWall(API_Coord point, API_Guid wall);
	static void SelectSingleWalls();
};

