// *****************************************************************************
// Source code for the Smartik ARCHICAD Add-On
// API Development Kit; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------



#include	"APIEnvir.h"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include "MeniFuncs.h"//Importing Meni fucntions class
#include	"ResourceIDs.h"
#include "APIEnvir.h" //Importing API environment
#include "TmrTech.h"
#include "TmrNavigator.h"
#include "TmrLayoutBook.h"
#include "TheRenderer.h"
#include "ShtilaLine.h"
#include "RotateOnAxis.h"
#include	"APICommon.h"
#include "Numberer.h"
#include "ViewMode.h"
#include	"UniString.hpp"
#include "SilentRender.h"
#include "LevelDimensions.h"
#include <cstring>
#include <string>
#include "Export3D.h"
#include "StairSection.h"
#include "FloorRec.h"
#include "RoomRec.h"
#include "TmrPoly.h"
#include "WallRec.h"
#include "MergeOldModel.h"
#include "Texturer.h"
// -----------------------------------------------------------------------------
// Handles menu commands
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL ExportHandler(const API_MenuParams* menuParams)
{
	switch (menuParams->menuItemRef.itemIndex) {
	case 1: TmrNavigator::Do_APINavigator(); break;
	case 2:	Export3D::Do_Export3D(); break;
	}
	return NoError;
}

GSErrCode __ACENV_CALL BasicHandler(const API_MenuParams* menuParams)
{
	switch (menuParams->menuItemRef.itemIndex) {
	case 1:	TheRenderer::Do_Renderer();	break;
	case 2:	Texturer().Do_APITexturer(); break;
	}

	return NoError;
}		// MenuCommandHandler

GSErrCode __ACENV_CALL ToolsHandler(const API_MenuParams* menuParams)
{

	switch (menuParams->menuItemRef.itemIndex) {
	case 1:	Numberer().Do_Numberer();	break;
	case 2:	TmrTech().Do_SelectInvertSameID();	break;
	case 3:	TmrTech().Do_SelectInvert();	break;
	case 4:	RotateOnAxis().Do_RotateElems();	break;
	case 5: FloorRec::SelectExternalWalls(); break;
	case 6: FloorRec::CalculateFloorArea(); break;
	case 7: RecHandler::Do_LeTlat(); break;
	case 8:	MergeOldModel::Do_MergeOldModel(); break;

	}
	return NoError;
}

GSErrCode __ACENV_CALL NaatHandler(const API_MenuParams* menuParams)
{
	switch (menuParams->menuItemRef.itemIndex) {
	case 1:	SilentRender().CheckServerFolder();	break;
	case 2: TmrTech::ShowSelectionGUID(); break;
	case 3: TmrTech::GetSelection(); break;
	}
	return NoError;
}

GSErrCode __ACENV_CALL DocHandler(const API_MenuParams* menuParams)
{

	switch (menuParams->menuItemRef.itemIndex) {
	case 1:	MeniFuncs().ProjNameAutoText();	break;
	case 2:	TmrLayoutBook().DeleteLayoutsOfEmptyFloors();	break;
	case 3:	LevelDimensions().AutoLevelDimensions();	break;
	case 4:	TmrLayoutBook().Do_AutoFitToGridCell();	break;
	case 5:RoomRec::Do_RoomWizard(); break;
	}
	return NoError;
}


// -----------------------------------------------------------------------------
// Dependency definitions
// -----------------------------------------------------------------------------

API_AddonType	__ACENV_CALL	CheckEnvironment(API_EnvirParams* envir)
{
	RSGetIndString(&envir->addOnInfo.name, ID_ADDON_INFO, 1, ACAPI_GetOwnResModule());
	RSGetIndString(&envir->addOnInfo.description, ID_ADDON_INFO, 2, ACAPI_GetOwnResModule());

	return APIAddon_Normal;
}		// CheckEnvironment


// -----------------------------------------------------------------------------
// Interface definitions
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL	RegisterInterface(void)
{
	GSErrCode err;
	err = ACAPI_Register_Menu(32500, 0, MenuCode_UserDef, MenuFlag_SeparatorBefore);
	err = ACAPI_Register_Menu(32501, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu(32502, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu(32503, 0, MenuCode_UserDef, MenuFlag_Default);
	err = ACAPI_Register_Menu(32504, 0, MenuCode_UserDef, MenuFlag_Default);
	ACAPI_Register_SupportedService('rend', 1);
	return err;
}		// RegisterInterface


// -----------------------------------------------------------------------------
// Called when the Add-On has been loaded into memory
// to perform an operation
// -----------------------------------------------------------------------------

GSErrCode	__ACENV_CALL Initialize(void)
{
	GSErrCode err;
	err = ACAPI_Install_MenuHandler(32500, ExportHandler);
	err = ACAPI_Install_MenuHandler(32501, BasicHandler);
	err = ACAPI_Install_MenuHandler(32502, ToolsHandler);
	err = ACAPI_Install_MenuHandler(32503, DocHandler);
	err = ACAPI_Install_MenuHandler(32504, NaatHandler);

	DBPrintf("Selection_Manager:: Initialize() ACAPI_Install_MenuHandler failed\n");
	err = ACAPI_Install_ModulCommandHandler('rend', 1, SilentRender::RenderCall_Handler);
	return err;
}		// Initialize


// -----------------------------------------------------------------------------
// FreeData
//		called when the Add-On is going to be unloaded
// -----------------------------------------------------------------------------

GSErrCode __ACENV_CALL	FreeData(void)
{
	return NoError;
}		// FreeData
