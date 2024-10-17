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
#include "RoomRec.h"
class ColRec
{
public:
	struct column;
	static void CreateColumnsFromDrawingPolys();
	static void CreateColumnElement(column col);
	static GS::Array<column> GetColumnPolys();

	static GS::Array<ColRec::column> GetColumnFills();

	static GS::Array<ColRec::column> DeleteDupColumns(GS::Array<ColRec::column> colArr);


};

