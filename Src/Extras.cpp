#include "Extras.h"
#include "APIEnvir.h"

#define	_Extras_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include "GeoAnalysis.h"
#include <algorithm>
using namespace std;




// ---------------------------------- Variables --------------------------------
void Extras::Do_CutFloorUnderStairs()
{
	GSErrCode           err;
	API_Element         stair, slab;
	GS::Array<API_Guid> stairList, slabList;
	err = ACAPI_Element_GetElemList(API_StairID, &stairList, APIFilt_OnVisLayer);
	if (err != NoError)
		return;
	for (GS::Array<API_Guid>::ConstIterator it = stairList.Enumerate(); it != nullptr; ++it)
	{
		BNZeroMemory(&stair, sizeof(API_Element));
		stair.header.guid = *it;
		err = ACAPI_Element_Get(&stair);
		err = ACAPI_Element_GetElemList(API_SlabID, &slabList, APIFilt_OnVisLayer);
		if (err != NoError)
			return;
		for (GS::Array<API_Guid>::ConstIterator ity = slabList.Enumerate(); ity != nullptr; ++ity)
		{
			BNZeroMemory(&slab, sizeof(API_Element));
			slab.header.guid = *ity;
			err = ACAPI_Element_Get(&slab);
			if (err != NoError)
				return;
			if (slab.header.floorInd + 1 == stair.header.floorInd)
			{
				//Extras::Do_CreateSlabHole(slab.header.guid, stair.header.guid);
				Extras::Do_CreateSlabHole(slab.header.guid, stair.header.guid);
			}
		}
	}
	return;
}

void Extras::Do_CreateSlabHole(API_Guid SlabG, API_Guid stairG)
{
	API_Element stair, slab;
	BNZeroMemory(&stair, sizeof(API_Element));
	BNZeroMemory(&slab, sizeof(API_Element));
	slab.header.guid = SlabG;
	stair.header.guid = stairG;
	GSErrCode err = ACAPI_Element_Get(&slab);
	err = ACAPI_Element_Get(&stair);
	API_ElementMemo slabMemo;
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));
	err = ACAPI_Element_GetMemo(slab.header.guid, &slabMemo);
	API_Box3D box3D;
	BNZeroMemory(&box3D, sizeof(API_Box3D));
	err = ACAPI_Database(APIDb_CalcBoundsID, &stair.header, &box3D);
	API_Coord A, B, C, D;
	A.x = box3D.xMin;
	A.y = box3D.yMin;
	B.x = box3D.xMax;
	B.y = box3D.yMin;
	C.x = box3D.xMax;
	C.y = box3D.yMax;
	D.x = box3D.xMin;
	D.y = box3D.yMax;

	API_Element poly;
	BNZeroMemory(&poly, sizeof(API_Element));

	poly.header.typeID = API_MeshID;
	API_ElementMemo pmemo;
	BNZeroMemory(&pmemo, sizeof(API_ElementMemo));
	err = ACAPI_Element_GetDefaults(&poly, &pmemo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (Polyline)", err);
		return;
	}

	poly.mesh.poly.nCoords = 5;
	poly.mesh.poly.nSubPolys = 1;
	poly.mesh.poly.nArcs = 0;
	pmemo.coords = (API_Coord**)BMAllocateHandle((poly.mesh.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	pmemo.pends = (Int32**)BMAllocateHandle((poly.mesh.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0);

	(*pmemo.pends)[0] = 0;
	(*pmemo.pends)[1] = 5;

	(*pmemo.coords)[0].x = 0.0;
	(*pmemo.coords)[0].y = 0.0;
	(*pmemo.coords)[1] = A;
	(*pmemo.coords)[2] = B;
	(*pmemo.coords)[3] = C;
	(*pmemo.coords)[4] = D;
	(*pmemo.coords)[5] = A;
	//err = ACAPI_CallUndoableCommand("Cut Stairs", [&]() -> GSErrCode { return ACAPI_Element_Create(&poly, &pmemo); });
	//if (err != NoError)
	//{
	//	ACAPI_WriteReport("Couldnt create holeshape", true);
	//	return;
	//}


	//API_Elem_Head** elemHead = (API_Elem_Head**)BMAllocateHandle(1 * sizeof(API_Elem_Head), ALLOCATE_CLEAR, 0);
	//(*elemHead)[0].guid = poly.header.guid;
	//err = ACAPI_CallUndoableCommand("Delete Poly", [&]() -> GSErrCode {
	//	return ACAPI_Element_Delete(elemHead, 1); });
	//if (err != NoError)
	//{
	//	ACAPI_WriteReport(GS::UniString("didnt delete"), true);
	//}


	err = ACAPI_Goodies(APIAny_InsertSubPolyID, &slabMemo, &pmemo);
	if (err == NoError) {
		API_ElementMemo tmpMemo;
		BNZeroMemory(&tmpMemo, sizeof(API_ElementMemo));
		tmpMemo.coords = slabMemo.coords;
		tmpMemo.pends = slabMemo.pends;
		tmpMemo.parcs = slabMemo.parcs;
		tmpMemo.vertexIDs = slabMemo.vertexIDs;
		tmpMemo.meshPolyZ = slabMemo.meshPolyZ;
		tmpMemo.edgeIDs = slabMemo.edgeIDs;
		tmpMemo.edgeTrims = slabMemo.edgeTrims;
		tmpMemo.contourIDs = slabMemo.contourIDs;
		slab.slab.poly.nCoords += 5;
		slab.slab.poly.nSubPolys += 1;
		err = ACAPI_CallUndoableCommand("Delete Poly", [&]() -> GSErrCode {
			return ACAPI_Element_ChangeMemo(slab.header.guid, APIMemoMask_Polygon, &tmpMemo); });
		if (err != NoError)
			ErrorBeep("ACAPI_Element_ChangeMemo", err);
	}
	else
		ErrorBeep("APIAny_InsertSubPolyID", err);

	ACAPI_DisposeElemMemoHdls(&pmemo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);


}

void Extras::Do_CreateSlabHoleEdit(API_Guid SlabG, API_Guid stairG)
{
	API_Element stair, slab;
	BNZeroMemory(&stair, sizeof(API_Element));
	BNZeroMemory(&slab, sizeof(API_Element));
	slab.header.guid = SlabG;
	stair.header.guid = stairG;
	GSErrCode err = ACAPI_Element_Get(&slab);
	err = ACAPI_Element_Get(&stair);
	API_ElementMemo slabMemo;
	BNZeroMemory(&slabMemo, sizeof(API_ElementMemo));
	err = ACAPI_Element_GetMemo(slab.header.guid, &slabMemo);
	API_Box3D box3D;
	BNZeroMemory(&box3D, sizeof(API_Box3D));
	err = ACAPI_Database(APIDb_CalcBoundsID, &stair.header, &box3D);
	API_Coord A, B, C, D;
	A.x = box3D.xMin;
	A.y = box3D.yMin;
	B.x = box3D.xMax;
	B.y = box3D.yMin;
	C.x = box3D.xMax;
	C.y = box3D.yMax;
	D.x = box3D.xMin;
	D.y = box3D.yMax;

	API_Element poly;
	BNZeroMemory(&poly, sizeof(API_Element));

	poly.header.typeID = API_MeshID;
	API_ElementMemo pmemo;
	BNZeroMemory(&pmemo, sizeof(API_ElementMemo));
	err = ACAPI_Element_GetDefaults(&poly, &pmemo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (Polyline)", err);
		return;
	}

	poly.mesh.poly.nCoords = 5;
	poly.mesh.poly.nSubPolys = 1;
	poly.mesh.poly.nArcs = 0;
	pmemo.coords = (API_Coord**)BMAllocateHandle((poly.mesh.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	pmemo.pends = (Int32**)BMAllocateHandle((poly.mesh.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0);

	(*pmemo.pends)[0] = 0;
	(*pmemo.pends)[1] = 5;

	(*pmemo.coords)[0].x = 0.0;
	(*pmemo.coords)[0].y = 0.0;
	(*pmemo.coords)[1] = A;
	(*pmemo.coords)[2] = B;
	(*pmemo.coords)[3] = C;
	(*pmemo.coords)[4] = D;
	(*pmemo.coords)[5] = A;


	int nC = slab.slab.poly.nCoords;
	int nSp = slab.slab.poly.nSubPolys;
	int nVIDs = BMGetHandleSize((GSHandle)slabMemo.vertexIDs) / sizeof(GS::UInt32);
	GS::UInt32 HighVertex = (*slabMemo.vertexIDs)[0];
	slabMemo.vertexIDs = (GS::UInt32**)BMReallocHandle((GSHandle)slabMemo.vertexIDs, (nVIDs + 5) * sizeof(GS::UInt32), 0, 0);
	slabMemo.coords = (API_Coord**)BMReallocHandle((GSHandle)slabMemo.coords, (nC + 5) * sizeof(API_Coord), 0, 0);
	slabMemo.pends = (Int32**)BMReallocHandle((GSHandle)slabMemo.pends, (nSp + 1) * sizeof(Int32), 0, 0);
	for (int i = 1; i < 6; i++)
		(*slabMemo.coords)[nC + i] = (*pmemo.coords)[i];
	(*slabMemo.pends)[nSp + 1] = nC + 5;
	for (int i = 1; i < 5; i++)
		(*slabMemo.vertexIDs)[nVIDs - 1 + i] = HighVertex + i;
	(*slabMemo.vertexIDs)[nVIDs + 4] = HighVertex + 1;
	(*slabMemo.vertexIDs)[0] = HighVertex + 5;

	slab.slab.poly.nCoords += 5;
	slab.slab.poly.nSubPolys += 1;
	err = ACAPI_CallUndoableCommand("Delete Poly", [&]() -> GSErrCode {
		return ACAPI_Element_Change(&slab, nullptr, &slabMemo, APIMemoMask_All, true); });
	if (err != NoError)
		ErrorBeep("ACAPI_Element_ChangeMemo", err);


	ACAPI_DisposeElemMemoHdls(&pmemo);
	ACAPI_DisposeElemMemoHdls(&slabMemo);


}

void Extras::ReClassifyColumns()
{

	GSErrCode           err;
	API_Element         element;
	GS::Array<API_Guid> columnList;
	API_ElementMemo memo;
	double               a, b;
	Int32			addParNum;
	API_LibPart libPart;
	API_AddParType** addPars = nullptr;
	err = ACAPI_Element_GetElemList(API_ObjectID, &columnList, APIFilt_OnVisLayer);
	if (err != NoError)
		return;
	for (GS::Array<API_Guid>::ConstIterator it = columnList.Enumerate(); it != nullptr; ++it)
	{ //for each object
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_All);
		if (err != NoError)
			return;//after retrieving relevant data

		BNZeroMemory(&libPart, sizeof(API_LibPart));
		libPart.index = element.object.libInd;
		ACAPI_LibPart_Get(&libPart);
		err = ACAPI_LibPart_GetParams(libPart.index, &a, &b, &addParNum, &addPars);
		//checking if this libpart is the one we'd like to reclassify;





	}

}

void Extras::OrganizeLayers() {
	//for each element Type(wall, floor, roof, window, door, stair, mesh, skylight, morph, object, railing, beam, curtainWall)
	//get all elements,
	//for each element
	//check if exists a layer called Type+ FloorName
	// if not - create one
	//add element to layer
	GSErrCode           err;
	API_Element         element, mask;
	GS::Array<API_Guid> elementList;
	API_ElementMemo memo;
	GS::UniString ObjectName;
	err = ACAPI_Element_GetElemList(API_ZombieElemID, &elementList, APIFilt_OnVisLayer);
	if (err != NoError)
		return;
	for (GS::Array<API_Guid>::ConstIterator it = elementList.Enumerate(); it != nullptr; ++it)
	{ //for each element
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);
		BNZeroMemory(&mask, sizeof(API_Element));
		ACAPI_ELEMENT_MASK_CLEAR(mask);
		BNZeroMemory(&memo, sizeof(API_ElementMemo));
		err = ACAPI_Element_GetMemo(element.header.guid, &memo, APIMemoMask_All);
		if (err != NoError)
			return;//after retrieving relevant data
		switch (element.header.typeID)
		{
		case API_WallID: ObjectName = GS::UniString("Wall"); break;
		case API_ColumnID: ObjectName = GS::UniString("Column"); break;
		case API_BeamID: ObjectName = GS::UniString("Beam"); break;
		case API_WindowID: ObjectName = GS::UniString("Window"); break;
		case API_DoorID: ObjectName = GS::UniString("Door"); break;
		case API_ObjectID: ObjectName = GS::UniString("Object"); break;
		case API_LampID: ObjectName = GS::UniString("Lamp"); break;
		case API_SlabID: ObjectName = GS::UniString("Slab"); break;
		case API_RoofID: ObjectName = GS::UniString("Roof"); break;
		case API_MeshID: ObjectName = GS::UniString("Mesh"); break;
		case API_CurtainWallID: ObjectName = GS::UniString("CurtainWall"); break;
		case API_ShellID: ObjectName = GS::UniString("Shell"); break;
		case API_SkylightID: ObjectName = GS::UniString("Skylight"); break;
		case API_MorphID: ObjectName = GS::UniString("Morph"); break;
		case API_StairID: ObjectName = GS::UniString("Stairs"); break;
		case API_RailingID: ObjectName = GS::UniString("Railing"); break;
		default: ObjectName = GS::UniString(""); break;
		}
		if (ObjectName != "")
		{
			int floInd = element.header.floorInd;
			GS::UniString FloorName = "";
			API_StoryInfo storyInfo;
			BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
			err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
			if (err != NoError)
				return;
			int n = BMhGetSize(reinterpret_cast<GSHandle> (storyInfo.data)) / sizeof(API_StoryType);
			for (int i = 0; i < n; i++)
			{
				if ((*storyInfo.data)[i].index == floInd)
				{
					FloorName = (*storyInfo.data)[i].uName;
					break;
				}

			}
			if (FloorName == "")
				return;
			GS::UniString LayerName = ObjectName + "_" + FloorName;
			API_Attribute Layer;
			BNZeroMemory(&Layer, sizeof(API_Attribute));
			Layer.header.typeID = API_LayerID;
			Layer.header.uniStringNamePtr = &LayerName;
			err = ACAPI_Attribute_Get(&Layer);
			if (err == APIERR_BADNAME)
			{
				err = ACAPI_Attribute_Create(&Layer, nullptr);
				if (err != NoError)
					return;
				err = ACAPI_Attribute_Get(&Layer);
			}
			else
			{
				if (err != NoError)
					return;
			}
			// add element to layer
			element.header.layer = Layer.header.index;
			ACAPI_ELEMENT_MASK_SET(mask, API_Elem_Head, layer);;
			err = ACAPI_Element_Change(&element, &mask, &memo, 0, true);
			if (err != NoError)
				return;
		}
	}
}

void	Extras::Do_CreateCircle(const API_Coord& point)
{
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));

	element.header.typeID = API_ArcID;
	GSErrCode err = ACAPI_Element_GetDefaults(&element, nullptr);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (Arc)", err);
		return;
	}

	element.arc.r = 2.0;
	element.arc.angle = 0.0;
	element.arc.ratio = 1.0;
	element.arc.origC = point;
	element.arc.linePen.penIndex = 3;
	element.arc.linePen.colorOverridePenIndex = 1;
	err = ACAPI_CallUndoableCommand("Delete Poly", [&]() -> GSErrCode {
		return ACAPI_Element_Create(&element, nullptr); });
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Create (Arc/Circle)", err);
	}
}	// Do_CreateCircle

////
// =============================================================================
//
// Required functions
//
// =============================================================================