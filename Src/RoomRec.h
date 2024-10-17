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
//#include	"GSRoot.hpp"
//#include "APIEnvir.h" //Importing API environment
//#include "APIdefs_Elements.h "
//#include "APIdefs_Automate.h"
#include	<DGModule.hpp>
//#include	"APIdefs_Database.h"
//#include "APIdefs_Registration.h"
//#include "APIdefs_Elements.h"
//#include "APIdefs_Callback.h"

class RoomRec
{
public:
	static void RecognizeRooms(bool WithText, bool WithFill, bool AutoNaming);
	static API_Guid GetClosestPoint(GS::Array<std::pair<API_Guid, API_Coord>> pair1, API_Coord point);
	static API_Coord RayCast(Line ray, Line hit);
	static GS::Array<API_Guid> DeleteDupGuids(GS::Array<API_Guid> cArr);
	static GS::Array<API_Guid> NewRaycastToWalls(API_Coord point, double raynum);
	static GS::Array<API_Coord> CreateIntresectArr(GS::Array<API_Guid> gArr);
	static void WallRayCastTest(bool WithText, bool WithFill, bool AutoNaming);
	static GS::Array <API_Coord>  CreateRoom(API_Coord c);
	static short CreateRandShort(short low, short high);
	static API_Coord GenRandCoord(double lower_bound, double upper_bound);
	static  short GetCurrentFloor();
	static API_Coord GenRandCoordAtRadius(API_Coord point, double Radius);

	static API_Coord GetArrMidPoint(GS::Array<API_Coord> cArr);

	static double DistanceOfFurthestPoint(GS::Array<API_Coord> cArr);

	static bool IsPointInPoly(API_Coord point, GS::Array<API_Coord> poly);

	static GS::Array<API_Guid> DeleteRepetetiveGuidsLow(GS::Array<API_Guid> gArr);

	static void RecognizeAllFloors(bool WithText, bool WithFill, bool AutoNaming);

	static void DeleteRoom();

	static void Do_RoomWizard(void);

	GSErrCode __ACENV_CALL APIRoomWizardPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr);

	static short DGCALLBACK APIRoomWizard_Handler(short message, short dialogID, short item, DGUserData, DGMessageData);


	static GS::Array<API_Guid> DeleteRepetetiveGuidsHigh(GS::Array<API_Guid> gArr);

};
