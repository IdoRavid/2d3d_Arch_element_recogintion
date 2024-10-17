// *****************************************************************************
// Header file for the Element Test Add-On
// API Development Kit 22; Mac/Win
//
// Namespaces:        Contact person:
//     -None-
//
// [SG compatible] - Yes
// *****************************************************************************
#define Numberer_H


// 
//
// General element functions
//
// 

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include "APIEnvir.h" //Importing API environment
#include "APIdefs_Elements.h "
#include "APIdefs_Automate.h"
#include	<DGModule.hpp>
#include	"APIdefs_Database.h"
#include "APIdefs_Registration.h"
#include "APIdefs_Elements.h"
#include "APIdefs_Callback.h"
class Numberer
{
public:

static void		NumberOpenings(GS::UniString input, bool door);
static void	Numberer::Do_Create3DText(API_Coord point, GS::UniString text, short floorInd, bool InsideWall);
static API_Coord Numberer::GetTextCoord(API_Coord begC, API_Coord endC, double distance, bool oSide, bool door, bool reflected);
static GSErrCode __ACENV_CALL	Numberer::APINumberPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/);
static short DGCALLBACK Numberer::APINumberer_Handler(short message, short dialogID, short item, DGUserData /*userData*/, DGMessageData /*msgData*/);
static void ManualNumberOpenings(GS::UniString input, bool door);
static void Numberer::Do_Numberer(void);
static void Numberer::ChangeParamValue(API_Element element1, const char* paramName, double val);
static void Numberer::ChangeParamValue(API_Element element1, const char* paramName, GS::UniString val);
};


