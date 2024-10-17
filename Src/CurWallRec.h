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
#include "WindowRec.h"
#include "WallMatch.h"
class CurWallRec
{
public:
	//Curved WallRec
	static void Do_CreateCurvedWall(double  angle, API_Coord begC, API_Coord endC, double width);
	static void CreateCurvedWallsFromNearbyDrawing();
	static void CreateWallFromArcPair(API_Element arc1, API_Element arc2);
	static GS::Array<std::pair<API_Element, API_Element>> DeleteDupGuidPairs(GS::Array<std::pair<API_Element, API_Element>> guidArr);

	static void DeleteArcFromUArc(API_Guid guid);
	static double GetWallAngleFromArc(API_Element arc);
	static void CreateCurvedWallsFromIndependentDrawing();
	static void CurvedWallsCreate();
	static GS::Array<GS::USize> FilterRelevantWallLines(GS::Array<GS::USize> IndArr);

	static double NewGetAngleFromArc(API_Element arc);

};
