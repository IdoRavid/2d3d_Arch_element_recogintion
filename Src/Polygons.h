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
#include <map>
//Class for polygons and Coords actions and info :)
class Polygons {
public:
	static bool isCoordInSimplePoly(API_Coord coord, std::vector<API_Coord> poly);

	static bool isCoordInPolygon(API_Coord coord, API_Coord** coords);

	static API_Coord centroidCoord(API_Coord** coords);

	static API_Coord centroidCoord(std::vector<API_Coord> coords);

	static API_Coord innerCentroid(API_Coord** coords,short floorInd=100);
	static bool IsCoordOnWall(API_Coord coord, short floorInd);

	static API_Coord** cArrTocoords(GS::Array<API_Coord> cArr);
};