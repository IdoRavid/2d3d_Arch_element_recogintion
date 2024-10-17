// *****************************************************************************
// Source code for the Element Test Add-On
// API Development Kit 22; Mac/Win
//
//	Main and common functions
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************
#include	"Numberer.h"
#include "APIEnvir.h"
#include "Extras.h"
#define	_Numberer_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include <cmath>
#include "TmrTexture.h"
#include "FloorRec.h"
#include "Resources.hpp"

using namespace std;


// ---------------------------------- Types ------------------------------------


// ---------------------------------- Variables --------------------------------



const short dialID = 32425;

int CurrWindow = 1;
int CurrDoor = 1;

API_Coord Numberer::GetTextCoord(API_Coord begC, API_Coord endC, double distance, bool oSide, bool door, bool reflected)
{
	double m = 0;
	double b = 0;
	double Yp = 0;
	double Xp = 0;
	ACAPI_WriteReport("begC.x is: %f", true, begC.x);
	ACAPI_WriteReport("begC.y is: %f", true, begC.y);
	ACAPI_WriteReport("endC.x is: %f", true, endC.x);
	ACAPI_WriteReport("endC.y is: %f", true, endC.y);
	if (endC.x == begC.x || abs(endC.x - begC.x) < 0.001)
	{
			Xp = endC.x;
		if (endC.y > begC.y)
		{
			Yp = begC.y + distance;
		}
		else
		{
			Yp = begC.y - distance;
		}

	}
	else {

		if (endC.y == begC.y || abs(endC.y - begC.y) < 0.001)
		{
			Yp = endC.y;
			if (endC.x > begC.x)
			{
				Xp = begC.x + distance;
			}
			else
			{
				Xp = begC.x - distance;
			}

		}
		else {
			m = ((endC.y - begC.y) / (endC.x - begC.x));
			b = (begC.y - (begC.x * m));


			double quadA = 1 + (m * m);
			double quadB = (2 * m * b - 2 * m * begC.y - 2 * begC.x);
			double quadC = (begC.x * begC.x) + (b - begC.y) * (b - begC.y) - (distance * distance);

			double Xp1 = (-quadB + sqrt((quadB * quadB) - 4 * quadA * quadC)) / (2 * quadA);
			double Xp2 = (-quadB - sqrt((quadB * quadB) - 4 * quadA * quadC)) / (2 * quadA);
			if (begC.x < endC.x)
			{
				if (begC.x < Xp1 && Xp1 < endC.x)
					Xp = Xp1;
				else
					Xp = Xp2;
			}

			if (begC.x > endC.x)
			{
				if (endC.x < Xp1 && Xp1 < begC.x)
					Xp = Xp1;
				else
					Xp = Xp2;
			}
			Yp = m * Xp + b;
		}
	}

	API_Coord DoorC;
	API_Coord TextC;
	DoorC.x = Xp;
	DoorC.y = Yp;
	ACAPI_WriteReport("DoorC.x is: %f", true, DoorC.x);
	ACAPI_WriteReport("DoorC.y is: %f", true, DoorC.y);
	TextC = DoorC;
	double TextDist = 1;
	if (endC.x == begC.x || abs(endC.x - begC.x) < 0.001)
	{
		//if false +x, if true -x
		if (oSide)
		{
			TextC.x = DoorC.x + TextDist - 0.5;
		}
		else
		{
			TextC.x = DoorC.x - TextDist - 0.5;
		}
	}
	else
	{

		if (endC.y == begC.y || abs(endC.y - begC.y) < 0.001)
		{
			// if false plus y, if true minus y
			if (oSide)
			{
				TextC.y = DoorC.y + TextDist - 0.25;
			}
			else
			{
				TextC.y = DoorC.y - TextDist - 0.5;
			}
		}

		else
		{
			double m2 = (-1) / (m);
			double b2 = DoorC.y - m2 * DoorC.x;


			double quadA2 = 1 + (m2 * m2);
			double quadB2 = (2 * m2 * b2 - 2 * m2 * DoorC.y - 2 * DoorC.x);
			double quadC2 = (DoorC.x * DoorC.x) + (b2 - DoorC.y) * (b2 - DoorC.y) - (TextDist * TextDist);

			double Xt1 = (-quadB2 + sqrt((quadB2 * quadB2) - 4 * quadA2 * quadC2)) / (2 * quadA2);
			double Xt2 = (-quadB2 - sqrt((quadB2 * quadB2) - 4 * quadA2 * quadC2)) / (2 * quadA2);
			// false +x, true -x
			if (m > 0)
			{
				if (Xt1 > Xt2)
				{
					if (oSide || (!door && reflected))
					{
						TextC.x = Xt2;
					}
					else
						TextC.x = Xt1;
				}
				else
				{
					if (oSide || (!door && reflected))
					{
						TextC.x = Xt1;
					}
					else
						TextC.x = Xt2;
				}

			}
			else
			{
				// false  -x, true +x
				if (Xt1 > Xt2)
				{
					if (oSide || (!door && reflected))
					{
						TextC.x = Xt1;
					}
					else
						TextC.x = Xt2;
				}
				else
				{
					if (oSide || (!door && reflected))
					{
						TextC.x = Xt2;
					}
					else
						TextC.x = Xt1;
				}
			}

			TextC.y = m2 * TextC.x + b2;
		}
	}
	ACAPI_WriteReport("TextC.x is: %f", true, TextC.x);
	ACAPI_WriteReport("TextC.y is: %f", true, TextC.y);
	if (sqrt((TextC.x-DoorC.x)* (TextC.x - DoorC.x) +(TextC.y - DoorC.y) * (TextC.y - DoorC.y)) > 7)
		return DoorC;
	else
		return TextC;
}

void	Numberer::Do_Create3DText(API_Coord point, GS::UniString text, short floorInd, bool InsideWall)
{
	GSErrCode err;
	//API_ParamOwnerType   paramOwner;
	//API_ChangeParamType  chgParam;
	//API_GetParamsType    getParams;
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	API_LibPart libPart;
	BNZeroMemory(&libPart, sizeof(libPart));
	double               a, b;
	Int32                addParNum;
	API_AddParType** addPars = nullptr;

	element.header.typeID = API_ObjectID;
	element.header.variationID = APIVarId_Object;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (InsideWall)
		GS::ucscpy(libPart.docu_UName, L("Text Opening S"));
	else
		GS::ucscpy(libPart.docu_UName, L("Text Opening L"));
	if (ACAPI_LibPart_Search(&libPart, false) != NoError) // find in the libraries,
	{
		ACAPI_WriteReport("Error in Finding library part: Text Opening ", true);
	}
	ACAPI_LibPart_Get(&libPart);
	err = ACAPI_LibPart_GetParams(libPart.index, &a, &b, &addParNum, &addPars);
	memo.params = addPars;
	element.object.libInd = libPart.index;
	element.object.pos = point;
	element.header.floorInd = floorInd;
	element.object.mat = 242;
	err = ACAPI_CallUndoableCommand("Create Text",
		[&]() -> GSErrCode {
			return ACAPI_Element_Create(&element, &memo);
		});
	if (err != NoError) {
		ACAPI_WriteReport("ACAPI_Element_Create (3d text)", true, err);
	}
	Numberer::ChangeParamValue(element, "rotAngleX", 0);
	Numberer::ChangeParamValue(element, "F", text);




}


void Numberer::NumberOpenings(GS::UniString input, bool door)
{
	ViewMode::SwitchTo2D();
	//get all doors
	GSErrCode           err;
	API_Element         element, wall;
	GS::Array<API_Guid> doorList;
	API_ElementMemo memo;
	int DoorNum = 1;
	API_ElemTypeID type;
	if (door)
		type = API_DoorID;
	else
		type = API_WindowID;
	err = ACAPI_Element_GetElemList(type, &doorList, APIFilt_OnVisLayer);
	if (err != NoError)
		return;
	for (GS::Array<API_Guid>::ConstIterator it = doorList.Enumerate(); it != nullptr; ++it)
	{ //for each door
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_All);
		if (err != NoError)
			return;
		//get door coords, find middle coord //find which direction is the door aimed towards, select MainCoord accordingly
		BNZeroMemory(&wall, sizeof(API_Element));
		wall.header.guid = element.door.owner;
		ACAPI_Element_Get(&wall);

		API_Coord Textloc = GetTextCoord(wall.wall.begC, wall.wall.endC, element.door.objLoc, element.door.openingBase.oSide, door, element.door.openingBase.reflected);
		string num = to_string(DoorNum);

		//input = GS::UniString("שפוצי", CC_Hebrew);
		GS::UniString text = input + GS::UniString(num.c_str()).ToUStr().Get();
		Do_Create3DText(Textloc, text, element.header.floorInd, !(FloorRec::IsWallExternal(wall.header.guid)));
		DoorNum++;
		if (door)
			CurrDoor = DoorNum;
		else CurrWindow = DoorNum;

		//create a 3d text object
		//change text paramater to count + prefix
		//instantiate the text object
	}
}

const short closeButton = 1, DoorText = 2, WindowText = 3, DoorLabel = 4, WindowLabel = 5, Number = 6, ManualDoor = 7, ManualWindow = 8, DoorVal = 9, WindowVal = 10;

void Numberer::Do_Numberer(void) {
	DGModelessInit(ACAPI_GetOwnResModule(), dialID, ACAPI_GetOwnResModule(), APINumberer_Handler, 0, 1);//Opening the obj export form.
	DGEnableMessage(dialID, DG_ALL_ITEMS, DG_MSG_DRAGDROP);
}

GSErrCode __ACENV_CALL	Numberer::APINumberPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	short dialID = (short)referenceID;
	switch (messageID) {
	case APIPalMsg_ClosePalette:		DGModelessClose(dialID);
		break;
	case APIPalMsg_HidePalette_Begin:	DGHideModelessDialog(dialID);
		break;
	case APIPalMsg_HidePalette_End:		DGShowModelessDialog(dialID, DG_DF_FIRST);
		break;
	case APIPalMsg_DisableItems_Begin:
	case APIPalMsg_DisableItems_End:	// actually do nothing, because the input finish functionality the buttons have to stay enabled
	case APIPalMsg_IsPaletteVisible:
	case APIPalMsg_OpenPalette:			break;
	}

	return NoError;
}		/* APINavigatorPaletteAPIControlCallBack */

short DGCALLBACK Numberer::APINumberer_Handler(short message, short dialogID, short item, DGUserData /*userData*/, DGMessageData /*msgData*/)
{
	short ret = 0;

	switch (message) {
	case DG_MSG_INIT:
	{//The action that happens when openning the Navigator 
		if (ACAPI_RegisterModelessWindow(dialogID, Numberer::APINumberPaletteAPIControlCallBack,
			API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
			API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout +
			API_PalEnabled_3D) != NoError) {
			DBPrintf("Navigator:: ACAPI_RegisterModelessWindow failed\n");
		}
		DGSetItemText(dialogID, DoorText, GS::UniString(L"ד", CC_Hebrew));
		DGSetItemText(dialogID, WindowText, GS::UniString(L"ח", CC_Hebrew));
		DGSetItemText(dialogID, DoorVal, GS::UniString(std::to_string(CurrDoor).c_str(), CC_Hebrew));
		DGSetItemText(dialogID, WindowVal, GS::UniString(std::to_string(CurrWindow).c_str(), CC_Hebrew));
	}
	case DG_MSG_CHANGE:
	{

	}

	case DG_MSG_CLICK:
	{
		switch (item) {
		case closeButton:
		{

			ret = item;
			break;
		}


		case Number:
		{
			CurrDoor = std::stoi(DGGetItemText(dialogID, DoorVal).ToCStr().Get());
			CurrWindow = std::stoi(DGGetItemText(dialogID, WindowVal).ToCStr().Get());
			NumberOpenings(DGGetItemText(dialogID, DoorText), true);
			NumberOpenings(DGGetItemText(dialogID, WindowText), false);
			DGSetItemText(dialogID, DoorVal, GS::UniString(std::to_string(CurrDoor).c_str(), CC_Hebrew));
			DGSetItemText(dialogID, WindowVal, GS::UniString(std::to_string(CurrWindow).c_str(), CC_Hebrew));
			break;
		}
		case ManualDoor:
		{
			CurrDoor = std::stoi(DGGetItemText(dialogID, DoorVal).ToCStr().Get());
			ManualNumberOpenings(DGGetItemText(dialogID, DoorText), true);
			DGSetItemText(dialogID, DoorVal, GS::UniString(std::to_string(CurrDoor).c_str(), CC_Hebrew));
			break;
		}
		case ManualWindow:
		{
			CurrWindow = std::stoi(DGGetItemText(dialogID, WindowVal).ToCStr().Get());
			ManualNumberOpenings(DGGetItemText(dialogID, WindowText), false);
			DGSetItemText(dialogID, WindowVal, GS::UniString(std::to_string(CurrWindow).c_str(), CC_Hebrew));
			break;
		}

		}

	case DG_MSG_TRACK:
		break;


	case DG_MSG_CLOSE: 
	{
		ACAPI_UnregisterModelessWindow(dialogID);
	}
	break;

	}


	case DG_MSG_UPDATE:
	{


	}
	}// APITexturer_Handler
	return ret;
}


void Numberer::ManualNumberOpenings(GS::UniString input, bool door)
{
	ViewMode::SwitchTo2D();
	int DoorNum = 1;
	if (door)
		DoorNum = CurrDoor;
	else DoorNum = CurrWindow;

	// create a polyline interface
	API_Coord			c;
	API_GetPolyType		polyInfo;
	GSErrCode			err;

	if (door)
		ACAPI_WriteReport(GS::UniString("Click on the Places you wish to place a  DOOR text"), true);
	else
		ACAPI_WriteReport(GS::UniString("Click on the Places you wish to place a  WINDOW text"), true);
	if (!ClickAPoint("Click on the places you wish to create a text", &c))
		return;

	BNZeroMemory(&polyInfo, sizeof(API_GetPolyType));
	polyInfo.useStandardPetPalette = true;
	strcpy_s(polyInfo.prompt, "Enter the next Text Location");
	polyInfo.method = APIPolyGetMethod_Polyline;
	polyInfo.startCoord.x = c.x;
	polyInfo.startCoord.y = c.y;

	err = ACAPI_Interface(APIIo_GetPolyID, &polyInfo, nullptr);
	if (err != NoError)
		return;
	int n = BMhGetSize(reinterpret_cast<GSHandle> (polyInfo.coords)) / sizeof(API_Coord);
	GS::Array<API_Coord> coords;
	for (int i = 0; i < n; ++i)
		coords.Push((*polyInfo.coords)[i]);

	//get polyline coords

	for (GS::UIndex ind = 1; ind < coords.GetSize(); ++ind)
	{ //for each coord

		string num = to_string(DoorNum);
		API_StoryInfo storyInfo;
		BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
		GS::UniString text = input + GS::UniString(num.c_str()).ToUStr().Get();

		BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
		Do_Create3DText(coords.Get(ind), text, storyInfo.actStory, false);
		DoorNum++;
		if (door)
			CurrDoor = DoorNum;
		else CurrWindow = DoorNum;

	}
}


void Numberer::ChangeParamValue(API_Element element1, const char* paramName, double val)
{
	API_Element         mask1;
	GSErrCode           err1 = NoError;
	API_ElementMemo Memo1;
	API_ToolBoxItem		tboxInfo;
	API_ParamOwnerType   paramOwner;
	API_ChangeParamType  chgParam;
	API_GetParamsType    getParams;


	BNZeroMemory(&element1, sizeof(element1.header.typeID)); //free up memory
	BNZeroMemory(&Memo1, sizeof(API_ElementMemo)); //save space for variables
	err1 = ACAPI_Element_Get(&element1); //get element
	err1 = ACAPI_Environment(APIEnv_GetToolBoxModeID, &tboxInfo, nullptr); //apply toolbox mode in order to edit
	if (err1 != NoError) {
		ACAPI_WriteReport("APIEnv_GetToolBoxMode", false);
	}

	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = 0;                 /* no library part */
	paramOwner.typeID = element1.header.typeID;      /* object element */
	paramOwner.guid = element1.header.guid;       /* element default */

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));
	
	err1 = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, nullptr);
	if (err1 != NoError)
		ACAPI_WriteReport("err2", false);

	BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));

	err1 = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, nullptr);
	if (err1 != NoError) {
		ACAPI_WriteReport("err3", false);
	}
	chgParam.index = 0;
	CHCopyC(paramName, chgParam.name);
	chgParam.realValue = val;
	err1 = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr);

	if (err1 != NoError) {
		ACAPI_WriteReport("err4", false);
	}
	err1 = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, nullptr);

	ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr);

	ACAPI_ELEMENT_MASK_CLEAR(mask1); //clear mask
	Memo1.params = getParams.params;

	if (err1 != NoError) {
		ACAPI_WriteReport("err5", false);
	}
	err1 = ACAPI_CallUndoableCommand("Change Element",
		[&]() -> GSErrCode {

			return  err1 = ACAPI_Element_Change(&element1, &mask1, &Memo1, APIMemoMask_AddPars, true); }); //use the changeparamaters function to "Save" our changes in the actual element - This time Using the memory mask add params to add the additional paramaters to the element

	if (err1 != NoError) {
		ACAPI_WriteReport("APIEnv_GetToolBoxMode", false);
		ACAPI_DisposeAddParHdl(&getParams.params);
	}

}

void Numberer::ChangeParamValue(API_Element element1, const char* paramName, GS::UniString val)
{
	API_Element         mask1;
	GSErrCode           err1 = NoError;
	API_ElementMemo Memo1;
	API_ToolBoxItem		tboxInfo;
	API_ParamOwnerType   paramOwner;
	API_ChangeParamType  chgParam;
	API_GetParamsType    getParams;


	BNZeroMemory(&element1, sizeof(element1.header.typeID)); //free up memory
	BNZeroMemory(&Memo1, sizeof(API_ElementMemo)); //save space for variables
	err1 = ACAPI_Element_Get(&element1); //get element
	err1 = ACAPI_Environment(APIEnv_GetToolBoxModeID, &tboxInfo, nullptr); //apply toolbox mode in order to edit
	if (err1 != NoError) {
		ACAPI_WriteReport("APIEnv_GetToolBoxMode", true);
	}

	BNZeroMemory(&paramOwner, sizeof(API_ParamOwnerType));
	paramOwner.libInd = 0;                 /* no library part */
	paramOwner.typeID = element1.header.typeID;      /* object element */
	paramOwner.guid = element1.header.guid;       /* element default */

	BNZeroMemory(&getParams, sizeof(API_GetParamsType));

	err1 = ACAPI_Goodies(APIAny_OpenParametersID, &paramOwner, nullptr);
	if (err1 != NoError)
		ACAPI_WriteReport("err2", true);

	BNZeroMemory(&chgParam, sizeof(API_ChangeParamType));

	err1 = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, nullptr);
	if (err1 != NoError) {
		ACAPI_WriteReport("err3", true);
	}
	chgParam.index = 0;
	CHCopyC(paramName, chgParam.name);
	GS::uchar_t	bufferUStr[API_UAddParStrLen];
	GS::ucsncpy(bufferUStr, GS::UniString(val.ToCStr()).ToUStr().Get(), API_UAddParStrLen - 1);
	chgParam.uStrValue = bufferUStr;
	err1 = ACAPI_Goodies(APIAny_ChangeAParameterID, &chgParam, nullptr);

	if (err1 != NoError) {
		TmrTech::errtoName(err1);
	}
	err1 = ACAPI_Goodies(APIAny_GetActParametersID, &getParams, nullptr);

	ACAPI_Goodies(APIAny_CloseParametersID, nullptr, nullptr);

	ACAPI_ELEMENT_MASK_CLEAR(mask1); //clear mask
	Memo1.params = getParams.params;

	if (err1 != NoError) {
		ACAPI_WriteReport("err5", true);
	}
	err1 = ACAPI_CallUndoableCommand("Change Element",
		[&]() -> GSErrCode {

			return  err1 = ACAPI_Element_Change(&element1, &mask1, &Memo1, APIMemoMask_AddPars, true); }); //use the changeparamaters function to "Save" our changes in the actual element - This time Using the memory mask add params to add the additional paramaters to the element

	if (err1 != NoError) {
		ACAPI_WriteReport("APIEnv_GetToolBoxMode", true);
		ACAPI_DisposeAddParHdl(&getParams.params);
	}

}


#ifdef __APPLE__
#pragma mark -
#endif