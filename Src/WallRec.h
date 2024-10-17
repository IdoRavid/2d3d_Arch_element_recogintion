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
#include "CurWallRec.h"
#include "DoorRec.h"
#include "ColRec.h"
#include"RecHandler.h"
class WallRec
{
public:
	//Variables
	static short WallRec::WallPenInd;



	//Wall Rec
	GS::Array<Line> static GetLineArr();
	static GS::Array < std::pair < Line, double >> GetWallLinePairs();
	static API_Guid CreateWallFromLine(Line line, double width);
	static void CreateWallsFromDrawingLines();
	static Line GetMidLine(Line lineA, Line lineB, bool MultiLine);
	static bool ParrallelUnconnectedLineTest(Line lineA, Line lineB);
	static short ReturnLinePenInd(API_Guid g);
	static void DeleteSuperFarWalls();
	static GS::Array < std::pair < Line, double> > WallRec::RemoveDupLineInd(GS::Array < std::pair < Line, double> > lArr, bool isWall, bool isWindow);


	

	///Unused

	//static void CreateWallsFromIntersectingLines();
	//static bool IsWallLineIntersection(Line line, API_Guid WallGuid);
	//static void TryCreatingWallFromUnidentified();
	//static GS::Array<Line> DeleteDupLines(GS::Array<Line> lineArr);
	//static double GetClosestPointDistance(Line lineA, Line lineB);
	//static int GetClosestPointCombination(Line lineA, Line lineB);
	//static bool SplitWallTest(Line lineA, Line lineB, Line lineC);
	
};

