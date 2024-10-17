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

class WindowRec
{
public:
	static GS::Array<Line> WindLines;
	static GS::Array<API_Element> WindArcs;
	static short WindPenInd;

	static void CreateWindowsFromDrawingLines();

	static GS::Array <Line> GetWindLinePairs();

	static GS::Array<GS::USize> FilterConvergentWindLines(GS::Array<GS::USize> IndArr);

	static Line GetWindowMidLine(GS::Array <GS::USize> TestLinesInd);

	static void CreateWallInWindLine(GS::Array < Line> WindMidLinePairs);

	static void CreatWindowFromLine(Line line);

	static GS::Array<Line> RemoveDupLineInd(GS::Array<Line> lArr, bool isWall, bool isWindow);


};

