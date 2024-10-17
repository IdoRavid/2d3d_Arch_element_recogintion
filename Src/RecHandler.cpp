#include "RecHandler.h"

GS::Array<Line> RecHandler::UL; //public dec
GS::Array<Line> RecHandler::WallLines;
GS::Array<API_Element> RecHandler::WallArcs;
GS::Array<API_Element> RecHandler::UArc;
GS::Array<API_Guid> RecHandler::UPoly;
GS::Array<API_Guid> RecHandler::UFill;

std::map<GS::UniString, GS::Array<API_AttributeIndex>> RecHandler::layerMap;  // map of layer Arrays, by name of the element type

void RecHandler::Do3DRec() {
	RecHandler::CreateArrays();
	RecHandler::IdentifyLayers();
	//walls
	WallRec::CreateWallsFromDrawingLines();
	CurWallRec::CurvedWallsCreate();
	//windows
	WindowRec::CreateWindowsFromDrawingLines();
	//doors
	DoorRec::CreateDoors();

	ColRec::CreateColumnsFromDrawingPolys();

	WallRec::DeleteSuperFarWalls();

	WallMatch::ManageWallMatching();

	FloorRec::CreateFloorElemFromContour(); // this function borrows from the RoomRecognition functions. it first recognizes the contour, then tries to create a slab from it


}

void RecHandler::CreateArrays() {
	RecHandler::CreteLlineArr();
	RecHandler::CreteArcArr();
	RecHandler::CretePolyArr();
	RecHandler::CreateFillArr();
}


void RecHandler::CreteLlineArr()
{
	//Creating An Array of non-identified Lines
	GS::Array<API_Guid> lineList;
	ACAPI_Element_GetElemList(API_LineID, &lineList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate(); it != nullptr; ++it)
	{ // for each line
		UL.Push(Line(*it));
	}
}

void RecHandler::CreteArcArr()
{//Creating An Array of non-identified Arcs
	GS::Array<API_Guid> lineList;
	ACAPI_Element_GetElemList(API_ArcID, &lineList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate(); it != nullptr; ++it)
	{ // for each line
		API_Element arc;
		BNZeroMemory(&arc, sizeof(API_Element));
		arc.header.guid = *it;
		ACAPI_Element_Get(&arc);
		UArc.Push(arc);
	}
}

void RecHandler::CretePolyArr()
{//Creating An Array of non-identified Polys
	GS::Array<API_Guid> lineList;
	ACAPI_Element_GetElemList(API_PolyLineID, &lineList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate(); it != nullptr; ++it)
	{ // for each line
		UPoly.Push(*it);
	}
}

void RecHandler::CreateFillArr()
{//Creating An Array of non-identified Polys
	GS::Array<API_Guid> lineList;
	ACAPI_Element_GetElemList(API_HatchID, &lineList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = lineList.Enumerate(); it != nullptr; ++it)
	{ // for each line
		RecHandler::UFill.Push(*it);
	}
}

GS::Array<API_AttributeIndex> RecHandler::FindInLayer(GS::UniString uni) {
	API_AttributeIndex size = 1;
	GS::Array<API_AttributeIndex> arr;
	GSErrCode err = ACAPI_Attribute_GetNum(API_LayerID, &size);
	for (API_AttributeIndex i = 1; i < size + 1; i++)
	{
		API_Attribute Layer;
		BNZeroMemory(&Layer, sizeof(API_Attribute));
		Layer.header.typeID = API_LayerID;
		Layer.header.index = i;
		GS::UniString  uniStringName;
		err = ACAPI_Attribute_Get(&Layer);
		//if (Layer.header.uniStringNamePtr != nullptr) {
		std::string str = Layer.header.name;
		GS::UniString string = GS::UniString((str.c_str()));
		if ((string).Contains(uni)) {
			arr.Push(i);
		}
		//}
	}
	return arr;
}

void RecHandler::IdentifyLayers()
{
	layerMap[GS::UniString("Wall")] = RecHandler::FindInLayer("wa");
	layerMap[GS::UniString("Window")] = RecHandler::FindInLayer("win");
	layerMap[GS::UniString("Door")] = RecHandler::FindInLayer("do");
	layerMap[GS::UniString("Stairs")] = RecHandler::FindInLayer("st");
	layerMap[GS::UniString("Columns")] = RecHandler::FindInLayer("col");
}



const short dialID = 32428;
const short closeButton = 1, layerPush = 2, WindowPush = 3, DoorPush = 4, ColPush = 5, WallMatchPush = 6, FloorPush = 7, GoButton = 8, ProgBar = 9;

void RecHandler::Do_LeTlat() {
	DGModelessInit(ACAPI_GetOwnResModule(), dialID, ACAPI_GetOwnResModule(), DuLeTlat_Handler, 0, 1);//Opening the obj export form.
	DGEnableMessage(dialID, DG_ALL_ITEMS, DG_MSG_DRAGDROP);
}

GSErrCode __ACENV_CALL	RecHandler::RecPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
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

short DGCALLBACK RecHandler::DuLeTlat_Handler(short message, short dialogID, short item, DGUserData /*userData*/, DGMessageData /*msgData*/)
{
	short ret = 0;

	switch (message) {
	case DG_MSG_INIT:
	{//The action that happens when openning the Navigator 
		if (ACAPI_RegisterModelessWindow(dialogID, RecHandler::RecPaletteAPIControlCallBack,
			API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
			API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_Layout +
			API_PalEnabled_3D) != NoError) {
			DBPrintf("Navigator:: ACAPI_RegisterModelessWindow failed\n");
		}

		DGSetItemValLong(dialID, layerPush, 1);
		DGSetItemValLong(dialID, WindowPush, 1);
		DGSetItemValLong(dialID, DoorPush, 1);
		DGSetItemValLong(dialID, ColPush, 1);
		DGSetItemValLong(dialID, WallMatchPush, 1);
		DGSetItemValLong(dialID, FloorPush, 1);
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


		case GoButton:
		{
			DGSetItemValLong(dialID, ProgBar, 0);
			RecHandler::CreateArrays();
			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);
			if (DGGetItemValLong(dialID, layerPush) == 1)
			{
				RecHandler::IdentifyLayers();
			}

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);
			//walls
			WallRec::CreateWallsFromDrawingLines();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

			CurWallRec::CurvedWallsCreate();



			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);
			//windows
			if (DGGetItemValLong(dialID, WindowPush) == 1)
				WindowRec::CreateWindowsFromDrawingLines();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);
			//doors
			if (DGGetItemValLong(dialID, DoorPush) == 1)
				DoorRec::CreateDoors();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

			if (DGGetItemValLong(dialID, ColPush) == 1)
				ColRec::CreateColumnsFromDrawingPolys();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

			WallRec::DeleteSuperFarWalls();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

			if (DGGetItemValLong(dialID, WallMatchPush) == 1)
				WallMatch::ManageWallMatching();

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

			if (DGGetItemValLong(dialID, FloorPush) == 1)
				FloorRec::CreateFloorElemFromContour(); // this function borrows from the RoomRecognition functions. it first recognizes the contour, then tries to create a slab from it

			DGSetItemValLong(dialID, ProgBar, DGGetItemValLong(dialID, ProgBar) + 1);

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
