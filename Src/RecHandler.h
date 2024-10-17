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
class RecHandler
{
public:


	static GS::Array<Line> UL; //Unidentified Lines
	static GS::Array<API_Element> UArc; //Unidentified Arcs
	static GS::Array<API_Guid> RecHandler::UPoly;
	static GS::Array<Line> RecHandler::WallLines;
	static GS::Array<API_Element> RecHandler::WallArcs;
	static GS::Array<API_AttributeIndex> RecHandler::WallLayer;
	static GS::Array<API_Guid> RecHandler::UFill;
	static std::map<GS::UniString, GS::Array<API_AttributeIndex>> RecHandler::layerMap;

	void static CreteLlineArr();
	void static CreteArcArr();
	static void CretePolyArr();
	static void CreateFillArr();

	static void Do3DRec();
	static void CreateArrays();
	static  GS::Array<API_AttributeIndex> FindInLayer(GS::UniString uni);
	static void IdentifyLayers();
	static void Do_LeTlat();
	static GSErrCode __ACENV_CALL RecPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr);
	static short DGCALLBACK DuLeTlat_Handler(short message, short dialogID, short item, DGUserData, DGMessageData);

};

