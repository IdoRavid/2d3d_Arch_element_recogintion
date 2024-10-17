#include "FloorRec.h"
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
#include <map>
#include "Numberer.h"
#include <random>
#include "TmrTech.h"
using namespace std;
bool randomUsed = false;
template<typename Numeric, typename Generator = std::mt19937>
Numeric random(Numeric from, Numeric to)
{
	thread_local static Generator gen(std::random_device{}());

	using dist_type = typename std::conditional
		<
		std::is_integral<Numeric>::value
		, std::uniform_int_distribution<Numeric>
		, std::uniform_real_distribution<Numeric>
		>::type;

	thread_local static dist_type dist;

	return dist(gen, typename dist_type::param_type{ from, to });
}



int FloorRec::CreateRandInt(int low, int high)
{
	return random<int>(low, high);
}

bool FloorRec::IsWallExternal(API_Guid wall)
{
	GS::Array<API_Guid> Exts = FloorRec::GetExternalWalls();
	for (GS::USize i = 0; i < Exts.GetSize(); i++)
	{
		if (Exts.Get(i) == wall)
			return true;
	}
	return false;
}


bool begB = true;
bool FloorRec::ChecIfWallExternal(API_Guid guid) {
	GS::Array<bool> MinX = { true,true,true };
	GS::Array<bool> MaxX = { true,true,true };
	GS::Array<bool> MinY = { true,true,true };
	GS::Array<bool> MaxY = { true,true,true };
	GS::Array< GS::Array<bool>> BoolArr = { MinX,MaxX, MinY, MaxY };
	GS::Array< GS::Array<bool>> FalseArr = { {false, false, false}, {false, false, false} , {false, false, false} , {false, false, false} };
	Line WallLine(guid);
	API_Coord MidCoord = { (WallLine.beg.x + WallLine.end.x) / 2,(WallLine.beg.y + WallLine.end.y) / 2 };
	GS::Array<API_Coord> WallCoords = { WallLine.beg, MidCoord, WallLine.end };

	GS::Array<API_Guid> WallList;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer| APIFilt_OnActFloor);
	for (GS::USize i = 0; i < WallList.GetSize(); ++i)
	{ // for each wall
		Line TestLine(WallList.Get(i));
		if (TestLine.guid != WallLine.guid)
		{
		for (GS::USize j = 0; j < WallCoords.GetSize(); ++j)
		{ // for each point
			
			API_Coord CurPoint = WallCoords.Get(j);
			//check minX
			API_Coord zeroP = { INFINITY, INFINITY };
			API_Coord HitPoint = FloorRec::RayCast(Line::CreateTangentByDeg(CurPoint, 180), TestLine);
			if (HitPoint.x != zeroP.x && HitPoint.y != zeroP.y && HitPoint.x < CurPoint.x)
				BoolArr[0][j] = false;
			// check maxX
			HitPoint = FloorRec::RayCast(Line::CreateTangentByDeg(CurPoint, 0), TestLine);
			if (HitPoint.x != zeroP.x && HitPoint.y != zeroP.y && HitPoint.x > CurPoint.x)
				BoolArr[1][j] = false;
			//check minY
			HitPoint =  FloorRec::RayCast(Line::CreateTangentByDeg(CurPoint, 270), TestLine);
			if (HitPoint.x != zeroP.x && HitPoint.y != zeroP.y && HitPoint.y < CurPoint.y)
				BoolArr[2][j] = false;
			// check maxY
			HitPoint = FloorRec::RayCast(Line::CreateTangentByDeg(CurPoint, 90), TestLine);
			if (HitPoint.x != zeroP.x && HitPoint.y != zeroP.y && HitPoint.y > CurPoint.y)
				BoolArr[3][j] = false;
		}
		}
		if (FalseArr == BoolArr)
			return false;
	}
	return true;
}

GS::Array<API_Guid> FloorRec::GetExternalWalls()
{
	GSErrCode err;
	GS::Array<API_Guid> WallList;
	GS::Array<API_Guid> ExtWalls;
	err = ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer| APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = WallList.Enumerate(); it != nullptr; ++it)
	{
		if (ChecIfWallExternal(*it))
			ExtWalls.Push(*it);
	}

	return ExtWalls;
}

void FloorRec::SelectExternalWalls()
{
	GS::Array<API_Neig> Sel;
	GS::Array<API_Guid> ExtWalls = GetExternalWalls();
	for (GS::USize i = 0; i < ExtWalls.GetSize(); i++)
	{
		Sel.Push(API_Neig(ExtWalls.Get(i)));
	}
	ACAPI_Element_Select(Sel, true);
}

void FloorRec::GetInternalWall() {

	SelectExternalWalls();
	TmrTech::Do_SelectInvertSameID();

}


Wall::Wall(API_Guid INguid) {
	API_Element wall;
	BNZeroMemory(&wall, sizeof(API_Element));
	guid = INguid;
	wall.header.guid = guid;
	ACAPI_Element_Get(&wall);
	begC = wall.wall.begC;
	endC = wall.wall.endC;
}

Wall::Wall() {
	API_Element wall;
	BNZeroMemory(&wall, sizeof(API_Element));
	guid = APINULLGuid;
	wall.header.guid = guid;
	begC.x = 0, begC.y = 0;
	endC.x = 0, endC.y = 0;

}

API_Guid FloorRec::GetClosestPoint(std::map<API_Guid, Wall> map1, API_Coord point) {
	double dist = 100000;
	API_Guid OurGuid = APINULLGuid;
	bool Isbeg = true;
	map<API_Guid, Wall>::iterator it;
	for (it = map1.begin(); it != map1.end(); it++)
	{
		API_Coord begC = it->second.begC;
		API_Coord endC = it->second.endC;
		double begdist = sqrt(((point.x - begC.x) * (point.x - begC.x)) + ((point.y - begC.y) * (point.y - begC.y)));
		double enddist = sqrt(((point.x - endC.x) * (point.x - endC.x)) + ((point.y - endC.y) * (point.y - endC.y)));
		if (begdist < dist) {
			OurGuid = it->first;
			Isbeg = true;
			dist = begdist;
		}
		if (enddist < dist) {
			OurGuid = it->first;
			Isbeg = false;
			dist = enddist;
		}
	}
	FloorRec::ChangebegBool(Isbeg);
	return OurGuid;
}

double FloorRec::GetElemArea(API_Guid guid) {
	API_ElementQuantity quantity;
	API_Quantities      quantities;
	API_QuantitiesMask  mask;
	API_QuantityPar   params;
	GSErrCode         err;

	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR(mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET(mask, hatch, surface);

	quantities.elements = &quantity;
	params.minOpeningSize = EPS;
	err = ACAPI_Element_GetQuantities(guid, &params, &quantities, &mask);
	if (err != NoError)
		return 0;
	else return quantities.elements->hatch.surface;

}

API_Guid FloorRec::CreateFloorPolygon() {
	std::map<API_Guid, Wall> wallMap;


	GS::Array<API_Guid> WallList, ContourList;
	ContourList = FloorRec::GetExternalWalls();
	// get Contour
	for (GS::Array<API_Guid>::ConstIterator it = ContourList.Enumerate(); it != nullptr; ++it)
		wallMap.emplace(*it, Wall(*it));
	// Create Map
	GS::Array<API_Coord> PolyArr;
	API_Coord LastPoint;
	PolyArr.Push(wallMap[ContourList.GetFirst()].begC); // insert first wall
	PolyArr.Push(wallMap[ContourList.GetFirst()].endC);
	LastPoint = wallMap[ContourList.GetFirst()].endC;
	wallMap.erase(ContourList.GetFirst());


	while (wallMap.size() > 0) {

		//find closest point to last point added
		API_Guid guid1 = FloorRec::GetClosestPoint(wallMap, LastPoint);
		//add closest point and pair point.
		if (begB) {
			PolyArr.Push(wallMap[guid1].begC);
			PolyArr.Push(wallMap[guid1].endC);
			LastPoint = wallMap[guid1].endC;
			wallMap.erase(guid1);
		}
		else {
			PolyArr.Push(wallMap[guid1].endC);
			PolyArr.Push(wallMap[guid1].begC);
			LastPoint = wallMap[guid1].begC;
			wallMap.erase(guid1);
		}
	}
	// continue until no more walls left in WallMap.
	API_Guid FillGuid = FloorRec::CreateFillFromCoordArr(PolyArr, 5);
	return FillGuid;
	//create fill

}

void FloorRec::CalculateFloorArea() {
	std::map<API_Guid, Wall> wallMap;
	//FloorRec::AutoConnectWalls(0.2);
	GSErrCode err;
	GS::Array<API_Guid> WallList, ContourList;
	ContourList = FloorRec::GetExternalWalls();
	// get Contour
	for (GS::Array<API_Guid>::ConstIterator it = ContourList.Enumerate(); it != nullptr; ++it)
		wallMap.emplace(*it, Wall(*it));
	// Create Map
	GS::Array<API_Coord> PolyArr;
	API_Coord LastPoint;
	PolyArr.Push(wallMap[ContourList.GetFirst()].begC); // insert first wall
	PolyArr.Push(wallMap[ContourList.GetFirst()].endC);
	LastPoint = wallMap[ContourList.GetFirst()].endC;
	wallMap.erase(ContourList.GetFirst());


	while (wallMap.size() > 0) {

		//find closest point to last point added
		API_Guid guid1 = FloorRec::GetClosestPoint(wallMap, LastPoint);
		//add closest point and pair point.
		if (begB) {
			PolyArr.Push(wallMap[guid1].begC);
			PolyArr.Push(wallMap[guid1].endC);
			LastPoint = wallMap[guid1].endC;
			wallMap.erase(guid1);
		}
		else {
			PolyArr.Push(wallMap[guid1].endC);
			PolyArr.Push(wallMap[guid1].begC);
			LastPoint = wallMap[guid1].begC;
			wallMap.erase(guid1);
		}
	}
	// continue until no more walls left in WallMap.
	API_Guid FillGuid = FloorRec::CreateFillFromCoordArr(PolyArr, 5);
	//create fill
	double area = FloorRec::GetElemArea(FillGuid);
	GS::UniString text = GS::UniString("Floor Area Is: ");
	text = text + GS::UniString(to_string(area).c_str());
	ACAPI_WriteReport(text, true);
	//report area size

	GS::Array<API_Guid> del;
	del.Push(FillGuid);
	err = ACAPI_CallUndoableCommand("Change Mesh",
		[&]() -> GSErrCode {
			return err = ACAPI_Element_Delete(del);
		});
	if (err != NoError)
		ACAPI_WriteReport("couldnt delete fill", true);
}

void FloorRec::ChangebegBool(bool val) {
	begB = val;
}

API_Guid FloorRec::CreateFillFromCoordArr(GS::Array<API_Coord> cArr, short pen) {
	if (cArr.GetSize() == 0)
		return APINULLGuid;

	GS::Array<API_Coord>  newArr = FloorRec::DeleteDupCoords(cArr);
	API_Element hatch;
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&hatch, sizeof(API_Element));
	GSErrCode err;

	hatch.header.typeID = API_HatchID;
	err = ACAPI_Element_GetDefaults(&hatch, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (hatch)", err);
		return APINULLGuid;
	}
	hatch.hatch.fillPen.Set(pen, pen);
	hatch.hatch.poly.nCoords = newArr.GetSize() + 1;
	hatch.hatch.poly.nSubPolys = 1;
	hatch.hatch.poly.nArcs = 0;
	memo.coords = (API_Coord * *)BMAllocateHandle((hatch.hatch.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.pends = (Int32 * *)BMAllocateHandle((hatch.hatch.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0);

	(*memo.pends)[0] = 0;
	(*memo.pends)[1] = newArr.GetSize() + 1;
	(*memo.coords)[0].x = 0.0;
	(*memo.coords)[0].y = 0.0;

	for (GS::USize i = 0; i < newArr.GetSize(); i++)
	{
		(*memo.coords)[int(i + 1)] = newArr.Get(i);
	}
	(*memo.coords)[int(newArr.GetSize() + 1)] = newArr.Get(0);
	err = ACAPI_CallUndoableCommand("Change Mesh",
		[&]() -> GSErrCode {
			return err = ACAPI_Element_Create(&hatch, &memo);
		});
	if (err != NoError) {
		ACAPI_WriteReport("Canceled!Couldn't Find Contour", false);
		return APINULLGuid;
	}
	return hatch.header.guid;

}

GS::Array<API_Coord> FloorRec::DeleteDupCoords(GS::Array<API_Coord> cArr) {
	if (cArr.GetSize() < 2)
		return cArr;
	for (int i = 0; i < int(cArr.GetSize());)
	{
		API_Coord Cur = cArr.Get(i);
		if (Cur.x == INFINITY && Cur.y == INFINITY) //checking validity
			cArr.Delete(i);
		else i++;
	}
	for (int i = 0; i < int(cArr.GetSize()) - 1;)
	{
		API_Coord Cur = cArr.Get(i);
		API_Coord nextC = cArr.Get(i + 1);
		double dist = sqrt(((Cur.x - nextC.x) * (Cur.x - nextC.x)) + ((Cur.y - nextC.y) * (Cur.y - nextC.y)));
		if (dist < 0.001)
		{
			cArr.Delete(i + 1);
		}
		else
			i++;
	}
	API_Coord Cur = cArr.GetFirst();
	API_Coord nextC = cArr.GetLast();
	double dist = sqrt(((Cur.x - nextC.x) * (Cur.x - nextC.x)) + ((Cur.y - nextC.y) * (Cur.y - nextC.y)));

	if (dist < 0.001)
		cArr.Delete(cArr.GetSize() - 1);
	return cArr;
}

GS::Array <API_Coord> FloorRec::CreateFloorcArr() {

	//FloorRec::AutoConnectWalls(0.2);
	std::map<API_Guid, Wall> wallMap;
	GS::Array<API_Guid> ContourList;
	ContourList = FloorRec::GetExternalWalls();
	// get Contour
	for (GS::Array<API_Guid>::ConstIterator it = ContourList.Enumerate(); it != nullptr; ++it)
		wallMap.emplace(*it, Wall(*it));
	// Create Map
	GS::Array<API_Coord> PolyArr;
	API_Coord LastPoint;
	PolyArr.Push(wallMap[ContourList.GetFirst()].begC); // insert first wall
	PolyArr.Push(wallMap[ContourList.GetFirst()].endC);
	LastPoint = wallMap[ContourList.GetFirst()].endC;
	wallMap.erase(ContourList.GetFirst());


	while (wallMap.size() > 0) {

		//find closest point to last point added
		API_Guid guid1 = FloorRec::GetClosestPoint(wallMap, LastPoint);
		//add closest point and pair point.
		if (begB) {
			PolyArr.Push(wallMap[guid1].begC);
			PolyArr.Push(wallMap[guid1].endC);
			LastPoint = wallMap[guid1].endC;
			wallMap.erase(guid1);
		}
		else {
			PolyArr.Push(wallMap[guid1].endC);
			PolyArr.Push(wallMap[guid1].begC);
			LastPoint = wallMap[guid1].begC;
			wallMap.erase(guid1);
		}
	}
	// continue until no more walls left in WallMap.
	return FloorRec::DeleteDupCoords(PolyArr);


}

void FloorRec::TryConnectWalls(API_Guid a, API_Guid b, double dist) {
	API_Element element1, element2, mask;
	BNZeroMemory(&element2, sizeof(API_Element));
	BNZeroMemory(&element1, sizeof(API_Element));

	element1.header.guid = a;
	element2.header.guid = b;
	ACAPI_Element_Get(&element1);
	ACAPI_Element_Get(&element2);
	bool BegC1 = false, BegC2 = false, ToConnect = false;

	if (Line::distanceBetweenPoints(element1.wall.begC, element2.wall.begC) < dist) {
		ToConnect = true;
		BegC1 = true;
		BegC2 = true;
	}
	if (Line::distanceBetweenPoints(element1.wall.begC, element2.wall.endC) < dist) {
		ToConnect = true;
		BegC1 = true;
		BegC2 = false;
	}
	if (Line::distanceBetweenPoints(element1.wall.endC, element2.wall.endC) < dist) {
		ToConnect = true;
		BegC1 = false;
		BegC2 = false;
	}
	if (Line::distanceBetweenPoints(element1.wall.endC, element2.wall.begC) < dist) {
		ToConnect = true;
		BegC1 = false;
		BegC2 = true;
	}

	if (ToConnect)
	{
		Line line1 = Line(element1.wall.begC, element1.wall.endC);
		Line line2 = Line(element2.wall.begC, element2.wall.endC);
		API_Coord c = Line::LineMeetingPoint(line1, line2);
		if (BegC1)
			element1.wall.begC = c;
		else
			element1.wall.endC = c;
		if (BegC2)
			element2.wall.begC = c;
		else
			element2.wall.endC = c;

		ACAPI_ELEMENT_MASK_CLEAR(mask);
		ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
		ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);
		GSErrCode err = ACAPI_CallUndoableCommand("Change Wall",
			[&]() -> GSErrCode {
				return err = ACAPI_Element_Change(&element1, &mask, nullptr, 0, true);
			});
		if (err != NoError)
			ACAPI_WriteReport("couldnt change wall1", false);
		err = ACAPI_CallUndoableCommand("Change Wall",
			[&]() -> GSErrCode {
				return err = ACAPI_Element_Change(&element2, &mask, nullptr, 0, true);
			});
		if (err != NoError)
			ACAPI_WriteReport("couldnt change wall2", false);
	}
}

void FloorRec::AutoConnectWalls(double dist) {

	GS::Array<API_Guid> WallList, WallList1;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer);

	for (GS::USize i = 0; i < WallList.GetSize(); i++)
	{
		ACAPI_Element_GetElemList(API_WallID, &WallList1, APIFilt_OnVisLayer);

		for (GS::USize j = 0; j < WallList1.GetSize(); j++)
		{
			API_Elem_Head wall1, wall2;
			BNZeroMemory(&wall1, sizeof(API_Elem_Head));
			BNZeroMemory(&wall2, sizeof(API_Elem_Head));
			wall1.guid = WallList.Get(i);
			wall2.guid = WallList1.Get(j);
			ACAPI_Element_GetHeader(&wall1);
			ACAPI_Element_GetHeader(&wall2);
			if (wall2.floorInd == wall1.floorInd && wall1.guid != wall2.guid) {
				FloorRec::TryConnectWalls(wall1.guid, wall2.guid, dist);
			}
		}
	}

}

void FloorRec::CreateFloorElemFromContour() {
	GS::Array<API_Guid>  ContourList;

	ContourList = FloorRec::GetExternalWalls();
	if (ContourList.GetSize() == 0)
		return;
	GS::Array<API_Coord> PolyArr;
	for (GS::USize i = 0; i < ContourList.GetSize() - 1; i++)
	{
		API_Coord point = Line::LineMeetingPoint(Line(ContourList.Get(i)), Line(ContourList.Get(i + 1)));
		PolyArr.Push(point);
	}
	API_Coord point = Line::LineMeetingPoint(Line(ContourList.GetLast()), Line(ContourList.GetFirst()));
	PolyArr.Push(point);
	FloorRec::CreateFloorFromCoordArr(PolyArr);
	//create fill

}


void FloorRec::CreateFloorFromCoordArr(GS::Array<API_Coord> cArr) {
	if (cArr.GetSize() == 0)
		return;

	GS::Array<API_Coord>  newArr = FloorRec::DeleteDupCoords(cArr);
	API_Element floor;
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&floor, sizeof(API_Element));
	GSErrCode err;

	floor.header.typeID = API_SlabID;
	err = ACAPI_Element_GetDefaults(&floor, &memo);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaults (hatch)", err);
		return;
	}

	floor.slab.poly.nCoords = newArr.GetSize() + 1;
	floor.slab.poly.nSubPolys = 1;
	floor.slab.poly.nArcs = 0;
	memo.coords = (API_Coord * *)BMAllocateHandle((floor.slab.poly.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	memo.pends = (Int32 * *)BMAllocateHandle((floor.slab.poly.nSubPolys + 1) * sizeof(Int32), ALLOCATE_CLEAR, 0);

	(*memo.pends)[0] = 0;
	(*memo.pends)[1] = newArr.GetSize() + 1;
	(*memo.coords)[0].x = 0.0;
	(*memo.coords)[0].y = 0.0;

	for (GS::USize i = 0; i < newArr.GetSize(); i++)
	{
		(*memo.coords)[int(i + 1)] = newArr.Get(i);
	}
	(*memo.coords)[int(newArr.GetSize() + 1)] = newArr.Get(0);
	err = ACAPI_CallUndoableCommand("Create floor",
		[&]() -> GSErrCode {
			return err = ACAPI_Element_Create(&floor, &memo);
		});
	if (err == APIERR_IRREGULARPOLY) {
		Int32 nResult = 0;
		API_RegularizedPoly** polys = nullptr;
		err = ACAPI_Goodies(APIAny_RegularizePolygonID, &floor.slab.poly, &nResult, polys);
		if (err != NoError || nResult >= 2)
			return;
		floor.slab.poly.nCoords = BMhGetSize(reinterpret_cast<GSHandle> ((*polys)[0].coords)) / sizeof(API_Coord) - 1;
		floor.slab.poly.nSubPolys = BMhGetSize(reinterpret_cast<GSHandle> ((*polys)[0].pends)) / sizeof(Int32) - 1;
		floor.slab.poly.nArcs = BMhGetSize(reinterpret_cast<GSHandle> ((*polys)[0].parcs)) / sizeof(API_PolyArc);

		memo.coords = (*polys)[0].coords;
		memo.pends = (*polys)[0].pends;
		memo.parcs = (*polys)[0].parcs;
		memo.vertexIDs = (*polys)[0].vertexIDs;
		err = ACAPI_CallUndoableCommand("Change Mesh",
			[&]() -> GSErrCode {
				return err = ACAPI_Element_Create(&floor, &memo);
			});
	}
}

std::pair<API_Guid, API_Guid> FloorRec::GetConnectedWallsPair(API_Guid wall)
{
	Line WallLine(wall);
	return std::make_pair(FloorRec::GetWallsNearPoint(WallLine.beg, wall), FloorRec::GetWallsNearPoint(WallLine.end, wall));
}

GS::Array<API_Guid> FloorRec::GetConnectedWallsArray(API_Guid wall)
{
	Line WallLine(wall);
	GS::Array<API_Guid> ConArr;
	double Sens = 0.2;
	GS::Array<API_Guid> WallList;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer);
	for (GS::USize i = 0; i < WallList.GetSize(); i++)
	{
		if (WallList.Get(i) != wall)
		{
			Line CurWall(WallList.Get(i));
			double distBeg = Line::distanceBetweenPoints(CurWall.beg, WallLine.beg);
			double distEnd = Line::distanceBetweenPoints(CurWall.end, WallLine.end);
			if (distBeg < Sens || distEnd < Sens)
			{
				ConArr.Push(CurWall.guid);
			}
		}
	}

	return ConArr;
}



API_Guid FloorRec::GetWallsNearPoint(API_Coord coord, API_Guid ourGuid)
{
	double Sens = 0.2;

	GS::Array<API_Guid> WallList, NearWalls;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer);
	for (GS::USize i = 0; i < WallList.GetSize(); i++)
	{
		if (WallList.Get(i) != ourGuid)
		{
			Line CurWall(WallList.Get(i));
			double distBeg = Line::distanceBetweenPoints(CurWall.beg, coord);
			double distEnd = Line::distanceBetweenPoints(CurWall.end, coord);
			if (distBeg < Sens || distEnd < Sens)
			{
				NearWalls.Push(CurWall.guid);
			}
		}
	}
	if (NearWalls.GetSize() == 1 || NearWalls.GetSize() > 2)
		return NearWalls.GetFirst();
	if (NearWalls.GetSize() == 2)
	{
		if (Line::GetRealLength(Line(NearWalls.Get(0))) > Line::GetRealLength(Line(NearWalls.Get(1))))
			return NearWalls.Get(0);
		else return NearWalls.Get(1);
	}
	return APINULLGuid;
}


GS::Array<API_Guid>  FloorRec::GetConnectingArray(API_Guid begWall, API_Guid endWall, bool IsBeg)
{
	GS::Array<API_Guid> ConArr;
	GS::Array<API_Guid> begArr = GetConnectedWallsArray(begWall);
	if (begArr.Contains(endWall))
	{
		if (IsBeg)
			return ConArr;
		else
			ConArr.Push(begWall);
	}
	else if (!begArr.Contains(endWall) && begArr.GetSize() > 0)
	{
		bool found = false;
		for (GS::USize i = 0; i < begArr.GetSize(); i++)
		{
			GS::Array<API_Guid> CurArr = FloorRec::GetConnectingArray(begArr.Get(i), endWall, false);
			if (CurArr.GetSize() == 1) {
				ConArr.Push(begWall);
				ConArr.Push(begArr.Get(i));
				found = true;
				break;
			}
			
		}
	}
	return FloorRec::DeleteDupGuids(ConArr);
}

GS::Array<API_Guid> FloorRec::DeleteDupGuids(GS::Array<API_Guid> Arr)
{
	if (Arr.GetSize() < 3)
		return Arr;
	for (GS::USize i = 0; i < Arr.GetSize(); i++)
	{
		for (GS::USize j = 0; j < Arr.GetSize();)
		{
			if (i != j && Arr.Get(i) == Arr.Get(j))
			{
				Arr.Delete(j);
			}
			else j++;
		}
	}
	return Arr;
}

API_Coord FloorRec::RayCast(Line ray, Line hit) {

	// case 1: on +-x axis; y is fixed;
	API_Coord zeroP;
	zeroP.x = INFINITY, zeroP.y = INFINITY;
	if (ray.ParallelToY && hit.ParallelToY) { // if both lines are parralel to y
		if (ray.beg.x == hit.beg.x)
			return ray.beg;
		else return zeroP;
	}
	else { // if they arent both parrallel to y
		if (ray.ParallelToX && hit.ParallelToX) { // if both lines are parralel to y
			if (ray.beg.y == hit.beg.y)
				return ray.beg;
			else return zeroP;
		}// If they are both parr to X, this if is irrelevant
		else {
			API_Coord MeetP = Line::LineMeetingPoint(ray, hit);
			if (Line::IsPointOnRealPartOfLine(MeetP, hit.beg, hit.end))
			{// only if the meeting point between the lines is on the real part of the second line
				return  MeetP;
			}
			else return zeroP;
		}
	}
}