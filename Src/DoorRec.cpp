#include "DoorRec.h"
#include "Numberer.h"

void DoorRec::CreateDoors() {
	DoorRec::CreateDoorsFromDrawingLines();
	DoorRec::CreateDoorsFromDrawingPolys();
}
void DoorRec::CreateDoorsFromDrawingLines()
{
	//iterate between Unidentified arcs
	// find an arc which has is connected to two lines 
	//- if one is  the same color, both of them should incapsulate a door

	//create a Door class which consists of the line and arc .
	//add it to Door arc array - decrement from UArcs
	GS::Array <Door>	DoorMidLinePairs = DoorRec::GetDoorLines();

	//for each Door in arc array - check if is nearby a wall
	//if so - change the wall so it takes the space of the door
	DoorRec::CreateWallInDoorLine(DoorMidLinePairs);

	//instantiate a door in the relevant location using the door class from the array
	for (GS::USize i = 0; i < DoorMidLinePairs.GetSize(); i++)
	{
		DoorRec::CreateDoorElement(DoorMidLinePairs.Get(i));
	}

}

void DoorRec::CreateDoorsFromDrawingPolys()
{
	//iterate between Unidentified arcs
	// find an arc which has is connected to two lines 
	//- if one is  the same color, both of them should incapsulate a door

	//create a Door class which consists of the line and arc .
	//add it to Door arc array - decrement from UArcs
	GS::Array <Door>	DoorMidLinePairs = DoorRec::GetDoorLinesFromPoly();

	//for each Door in arc array - check if is nearby a wall
	//if so - change the wall so it takes the space of the door
	DoorRec::CreateWallInDoorLine(DoorMidLinePairs);

	//instantiate a door in the relevant location using the door class from the array
	for (GS::USize i = 0; i < DoorMidLinePairs.GetSize(); i++)
	{
		DoorRec::CreateDoorElement(DoorMidLinePairs.Get(i));
	}

}

GS::Array <Door> DoorRec::GetDoorLinesFromPoly() {
	GS::Array<Door> dArr;

	for (GS::USize i = 0; i < RecHandler::UArc.GetSize();) { // for each arc
		API_Element CurArc = RecHandler::UArc.Get(i);
		bool created2 = false;
		for (GS::USize j = 0; j < RecHandler::UPoly.GetSize();)
		{//for each Unidentified Line
			bool created = false;

			API_Element CurPoly;
			BNZeroMemory(&CurPoly, sizeof(API_Element));
			CurPoly.header.guid = RecHandler::UPoly.Get(j);
			ACAPI_Element_Get(&CurPoly);

			if (CurPoly.polyLine.poly.nCoords > 3 && CurPoly.polyLine.poly.nCoords < 7) {
				GS::Array<Line> PolyLines = Line::GetLinesFromPoly(CurPoly);
				for (GS::USize t = 0; t < PolyLines.GetSize(); )
				{
					bool created3 = false;

					Line TestLine = PolyLines.Get(t);
					if (TestLine.PenInd == CurArc.arc.linePen.penIndex)
					{
						if (TestLine.layer == CurArc.header.layer) {
							if (DoorRec::GetDoorAngleFromArc(CurArc) > 0)
							{
								if ((RecHandler::layerMap[GS::UniString("Door")].GetSize() == 0) || (RecHandler::layerMap[GS::UniString("Door")].Contains(TestLine.layer) && RecHandler::layerMap[GS::UniString("Door")].Contains(CurArc.header.layer)) || (1 == CurArc.header.layer && 1 == TestLine.layer))
								{
									if (!DoorRec::IsConnectedToArc(i)) {
										Door door = DoorRec::CheckIfDoor(CurArc, TestLine);
										if (door.OpeningCoord.x != INFINITY) {
											if (Line::GetRealLength(door.DoorLine) > 0.5) {
												{
													created = true;
													created2 = true;
													created3 = true;
													dArr.Push(door);
													RecHandler::UPoly.Delete(j);
													RecHandler::UArc.Delete(i);
													break; // the t for;
												}
											}
										}
									}
								}
							}

						}
					}
					if (!created3)
						t++;
				}
			}
			if (!created)
				j++;

		}
		if (!created2)
			i++;
	}
	return dArr;
}


GS::Array <Door> DoorRec::GetDoorLines() {
	GS::Array<Door> dArr;

	for (GS::USize i = 0; i < RecHandler::UArc.GetSize();) { // for each arc
		API_Element CurArc = RecHandler::UArc.Get(i);
		bool created2 = false;
		for (GS::USize j = 0; j < RecHandler::UL.GetSize(); j++)
		{//for each Unidentified Line
			Line TestLine = RecHandler::UL.Get(j);
			if (TestLine.PenInd == CurArc.arc.linePen.penIndex)
			{
				if (TestLine.layer == CurArc.header.layer) {
					if (DoorRec::GetDoorAngleFromArc(CurArc) > 0)
					{
						if ((RecHandler::layerMap[GS::UniString("Door")].GetSize() == 0) || (RecHandler::layerMap[GS::UniString("Door")].Contains(TestLine.layer) && RecHandler::layerMap[GS::UniString("Door")].Contains( CurArc.header.layer)) || (1 == CurArc.header.layer && 1 == TestLine.layer))
						{
							if (!DoorRec::IsConnectedToArc(i)) {
								Door door = DoorRec::CheckIfDoor(CurArc, TestLine);
								if (door.OpeningCoord.x != INFINITY) {
									if (Line::GetRealLength(door.DoorLine) > 0.5) {
										{
											created2 = true;
											dArr.Push(door);
											RecHandler::UL.Delete(j);
											RecHandler::UArc.Delete(i);
											break;
										}
									}
								}
							}
						}
					}

				}
			}
		}
		if (!created2)
			i++;
	}
	return dArr;
}

void DoorRec::CreateWallInDoorLine(GS::Array <Door> dArr) {
	for (GS::USize i = 0; i < dArr.GetSize(); i++)
	{
		Line CurLine = dArr.Get(i).DoorLine;
		GS::Array<API_Guid> WallList;
		ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
		for (GS::USize j = 0; j < WallList.GetSize(); j++)
		{
			API_Element element, mask;
			API_Coord NewCoord;
			GSErrCode err;
			BNZeroMemory(&element, sizeof(API_Element));
			element.header.guid = WallList.Get(j);
			ACAPI_Element_Get(&element);
			//if change beg
			Line WallLine = Line(element.wall.begC, element.wall.endC);
			if (Line::distanceBetweenPoints(element.wall.begC, CurLine.beg) < 0.1 || Line::distanceBetweenPoints(element.wall.begC, CurLine.end) < 0.1)
			{
				if (Line::distanceBetweenPoints(element.wall.begC, CurLine.beg) < 0.1)
				{
					NewCoord = CurLine.end;;
					if (!WallLine.IsPointOnLine(NewCoord, 0.009))
					{
						//if new beg point isnt exactly on line, find the closest point on the line.
						NewCoord = Line::GetClosesetPointOnline(WallLine, NewCoord);
					}
					ACAPI_ELEMENT_MASK_CLEAR(mask);//Clearing mask
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
					element.wall.begC = NewCoord;
					err = ACAPI_CallUndoableCommand("Change Wall",
						[&]() -> GSErrCode {
							return ACAPI_Element_Change(&element, &mask, nullptr, 1, true);
						});
					break;
				}
				else
				{
					NewCoord = CurLine.beg;
					if (!WallLine.IsPointOnLine(NewCoord, 0.009))
					{
						//if new beg point isnt exactly on line, find the closest point on the line.
						NewCoord = Line::GetClosesetPointOnline(WallLine, NewCoord);
					}

					ACAPI_ELEMENT_MASK_CLEAR(mask);//Clearing mask
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, begC);
					element.wall.begC = NewCoord;
					err = ACAPI_CallUndoableCommand("Change Wall",
						[&]() -> GSErrCode {
							return ACAPI_Element_Change(&element, &mask, nullptr, 1, true);
						});
					break;
				}

			}
			//if change end;
			if (Line::distanceBetweenPoints(element.wall.endC, CurLine.beg) < 0.1 || Line::distanceBetweenPoints(element.wall.endC, CurLine.end) < 0.1)
			{
				if (Line::distanceBetweenPoints(element.wall.endC, CurLine.beg) < 0.1)
				{
					NewCoord = CurLine.end;
					if (!WallLine.IsPointOnLine(NewCoord, 0.009))
					{
						//if new beg point isnt exactly on line, find the closest point on the line.
						NewCoord = Line::GetClosesetPointOnline(WallLine, NewCoord);
					}
					ACAPI_ELEMENT_MASK_CLEAR(mask);//Clearing mask
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);
					element.wall.endC = NewCoord;
					err = ACAPI_CallUndoableCommand("Change Wall",
						[&]() -> GSErrCode {
							return ACAPI_Element_Change(&element, &mask, nullptr, 1, true);
						});
					break;
				}
				else
				{
					NewCoord = CurLine.beg;
					if (!WallLine.IsPointOnLine(NewCoord, 0.009))
					{
						//if new beg point isnt exactly on line, find the closest point on the line.
						NewCoord = Line::GetClosesetPointOnline(WallLine, NewCoord);
					}
					ACAPI_ELEMENT_MASK_CLEAR(mask);//Clearing mask
					ACAPI_ELEMENT_MASK_SET(mask, API_WallType, endC);
					element.wall.endC = NewCoord;
					err = ACAPI_CallUndoableCommand("Change Wall",
						[&]() -> GSErrCode {
							return ACAPI_Element_Change(&element, &mask, nullptr, 1, true);
						});
					break;
				}

			}

		}

	}
}

void 	DoorRec::CreateDoorElement(Door door) {
	API_Element			element;
	API_Guid wallGuid = APINULLGuid;
	API_Coord BegC;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	GSErrCode			err = NoError;
	API_AddParType** markAddPars;
	double DoorWidth = Line::GetRealLength(door.DoorLine);
	API_Coord DoorCenter;
	DoorCenter.x = (door.DoorLine.beg.x + door.DoorLine.end.x) / 2;
	DoorCenter.y = (door.DoorLine.beg.y + door.DoorLine.end.y) / 2;

	GS::Array<API_Guid> WallList;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::USize j = 0; j < WallList.GetSize(); j++)
	{
		element;
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = WallList.Get(j);
		ACAPI_Element_Get(&element);

		if (Line::IsPointOnRealPartOfLine(DoorCenter, element.wall.begC, element.wall.endC))
		{
			BegC = element.wall.begC;
			wallGuid = element.header.guid;
			break;
		}
	}

	if (wallGuid == APINULLGuid) {
		Line inverse(door.DoorLine.end, door.DoorLine.beg);
		wallGuid = WallRec::CreateWallFromLine(inverse, 0.2);
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = wallGuid;
		ACAPI_Element_Get(&element);
		BegC = element.wall.begC;
	}



	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = wallGuid;
	ACAPI_Element_Get(&element);
	if (!Line::AreLinesInSameDirection(Line(element.wall.begC, element.wall.endC), door.DoorLine))
	{
		door = Door(Line(door.DoorLine.end, door.DoorLine.beg), door.OpeningCoord);
	}
	element.header.typeID = API_DoorID;
	marker.subType = APISubElemMemoMask_MainMarker;

	API_LibPart libPart;
	BNClear(libPart);

	err = ACAPI_Element_GetDefaultsExt(&element, &memo, 1UL, &marker);
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_GetDefaultsExt", err);
		ACAPI_DisposeElemMemoHdls(&memo);
		ACAPI_DisposeElemMemoHdls(&marker.memo);
		return;
	}

	delete libPart.location;
	libPart.index = 0;
	double a, b;
	Int32 addParNum;
	BNZeroMemory(&libPart, sizeof(libPart));
	GS::ucscpy(libPart.docu_UName, L("Door 23"));
	if (ACAPI_LibPart_Search(&libPart, false) != NoError) // find door 21 in the libraries,
	{
		ACAPI_WriteReport("Error in Finding that library part: ", false);
	}
	ACAPI_LibPart_Get(&libPart);
	err = ACAPI_LibPart_GetParams(libPart.index, &a, &b, &addParNum, &markAddPars);

	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;
	element.door.objLoc = DistCPtr(&DoorCenter, &BegC);
	element.door.owner = wallGuid;

	std::pair<bool, bool> boolpair = Door::ConfigDoorBools(door.DoorLine, door.OpeningCoord, wallGuid);

	element.door.openingBase.reflected = boolpair.first;
	element.door.openingBase.oSide = boolpair.second;
	err = ACAPI_CallUndoableCommand("Create Door",
		[&]() -> GSErrCode {
			return 	err = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
		});
	if (err != NoError)
		ErrorBeep("ACAPI_Element_CreateExt", err);



	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);

	Numberer::ChangeParamValue(element, "A", DoorWidth + 0.05); //WIDTH

	return;
}

double DoorRec::GetDoorAngleFromArc(API_Element arc) {
	double smallAng = std::min(arc.arc.begAng, arc.arc.endAng);
	double LargeAng = std::max(arc.arc.begAng, arc.arc.endAng);
	double deg = (LargeAng - smallAng) * 180.0 / PI;
	if (deg > 90)
		deg -= 90;
	return abs(deg);

}

Door DoorRec::CheckIfDoor(API_Element arc, Line line) {

	double sensitivity = 0.2;
	if (Line::distanceBetweenPoints(Line::GetArcBegCoord(arc), line.beg) < sensitivity)
	{
		bool perp = Line::IsPerp(Line(Line::GetArcBegCoord(arc), line.end), Line(Line::GetArcEndCoord(arc), line.end));
		if (perp)
			return Door(Line(Line::GetArcEndCoord(arc), line.end), line.beg);
		if (!perp && abs((Line::GetRealLength(line) - arc.arc.r)) < 0.3)
			return Door(Line(Line::GetArcEndCoord(arc), line.end), line.beg);
	}
	if (Line::distanceBetweenPoints(Line::GetArcEndCoord(arc), line.beg) < sensitivity)
	{
		bool perp = Line::IsPerp(Line(Line::GetArcBegCoord(arc), line.end), Line(Line::GetArcEndCoord(arc), line.end));
		if (perp)
			return Door(Line(Line::GetArcBegCoord(arc), line.end), line.beg);
		if (!perp && abs((Line::GetRealLength(line) - arc.arc.r)) < 0.3)
			return Door(Line(Line::GetArcBegCoord(arc), line.end), line.beg);
	}
	if (Line::distanceBetweenPoints(Line::GetArcBegCoord(arc), line.end) < sensitivity)
	{
		bool perp = Line::IsPerp(Line(Line::GetArcBegCoord(arc), line.beg), Line(Line::GetArcEndCoord(arc), line.beg));
		if (perp)
			return Door(Line(Line::GetArcEndCoord(arc), line.beg), line.end);
		if (!perp && abs((Line::GetRealLength(line) - arc.arc.r)) < 0.3)
			return Door(Line(Line::GetArcEndCoord(arc), line.beg), line.end);
	}
	if (Line::distanceBetweenPoints(Line::GetArcEndCoord(arc), line.end) < sensitivity)
	{
		bool perp = Line::IsPerp(Line(Line::GetArcBegCoord(arc), line.beg), Line(Line::GetArcEndCoord(arc), line.beg));
		if (perp)
			return Door(Line(Line::GetArcBegCoord(arc), line.beg), line.end);
		if (!perp && abs((Line::GetRealLength(line) - arc.arc.r)) < 0.3)
			return Door(Line(Line::GetArcBegCoord(arc), line.beg), line.end);
	}
	return Door();
}




Door::Door()
{
	DoorLine = Line();
	OpeningCoord.x = INFINITY, OpeningCoord.y = INFINITY;
	reflected = false;
	oSide = false;
}

Door::Door(Line line, API_Coord coord)
{
	DoorLine = line;
	OpeningCoord = coord;
	reflected = false;
	oSide = false;
}

std::pair<bool, bool> Door::ConfigDoorBools(Line line, API_Coord coord, API_Guid WallGuid) {
	std::pair<bool, bool> boolpair = std::make_pair(false, false);

	API_Element wall;
	BNZeroMemory(&wall, sizeof(API_Element));
	wall.header.guid = WallGuid;
	ACAPI_Element_Get(&wall);
	Line WallLine = Line(wall.wall.begC, wall.wall.endC);
	bool WallDir = Line::IsLineInPositiveDir(WallLine);
	bool closerToLeft = (abs(std::min(line.beg.x, line.end.x) - coord.x) > abs(std::max(line.beg.x, line.end.x) - coord.x));
	bool closerUp = (abs(std::min(line.beg.y, line.end.y) - coord.y) > abs(std::max(line.beg.y, line.end.y) - coord.y));


	if (WallLine.ParallelToY) {
		if (WallDir)
		{
			if (closerUp) {
				if (coord.x > line.beg.x) {
					boolpair.first = true;
					boolpair.second = false;

				}
				else {
					boolpair.first = false;
					boolpair.second = true;


				}
			}
			else {
				if (coord.x > line.beg.x) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = true;


				}
			}
		}
		else
		{ //line.beg.x > line.end.x

			if (closerUp) {
				if (coord.x > line.beg.x) {
					boolpair.first = true;
					boolpair.second = true;

				}
				else {
					boolpair.first = false;
					boolpair.second = false;


				}
			}
			else {
				if (coord.x > line.end.x) {
					boolpair.first = false;
					boolpair.second = true;

				}
				else {
					boolpair.first = true;
					boolpair.second = false;


				}
			}
		}


	}

	if (WallLine.ParallelToX) {
		if (WallDir)
		{
			if (closerToLeft) {
				if (coord.y > line.beg.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = true;


				}
			}
			else {
				if (coord.y > line.beg.y) {
					boolpair.first = true;
					boolpair.second = false;

				}
				else {
					boolpair.first = false;
					boolpair.second = true;


				}
			}
		}
		else
		{ //line.beg.x > line.end.x

			if (closerToLeft) {
				if (coord.y > line.beg.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = true;


				}
			}
			else {
				if (coord.y > line.end.y) {
					boolpair.first = false;
					boolpair.second = true;

				}
				else {
					boolpair.first = true;
					boolpair.second = false;


				}
			}
		}


	}

	if (line.m > 0) {
		if (WallDir)
		{
			if (closerToLeft) {
				if (coord.y > line.beg.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = false;
					boolpair.second = false;


				}
			}
			else {
				if (coord.y > line.beg.y) {
					boolpair.first = true;
					boolpair.second = true;

				}
				else {
					boolpair.first = false;
					boolpair.second = true;


				}
			}
		}
		else
		{ //line.beg.x > line.end.x

			if (closerToLeft) {
				if (coord.y > line.beg.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = true;


				}
			}
			else {
				if (coord.y > line.end.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = false;


				}
			}
		}


	}

	if (line.m < 0) {
		if (WallDir)
		{
			if (closerUp) {
				if (coord.y > line.beg.y) {
					boolpair.first = true ;
					boolpair.second = true;

				}
				else {
					boolpair.first = false;
					boolpair.second = false;


				}
			}
			else {
				if (coord.y > line.beg.y) {
					boolpair.first = true;
					boolpair.second = true;

				}
				else {
					boolpair.first = false;
					boolpair.second = false;


				}
			}
		}
		else
		{ //line.beg.x > line.end.x

			if (closerUp) {
				if (coord.y > line.beg.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = false;
					boolpair.second = true;


				}
			}
			else {
				if (coord.y > line.end.y) {
					boolpair.first = false;
					boolpair.second = false;

				}
				else {
					boolpair.first = true;
					boolpair.second = true;


				}
			}
		}


	}


	if (Line::IsLineInPositiveDir(line) && line.ParallelToX) {
		boolpair.second = !boolpair.second;
	}
	return boolpair;
}

bool DoorRec::IsConnectedToArc(GS::UIndex arcInd) {
	API_Element CurArc = RecHandler::UArc.Get(arcInd);
	API_Coord BegCur = Line::GetArcBegCoord(CurArc);
	API_Coord EndCur = Line::GetArcEndCoord(CurArc);
	for (GS::USize i = 0; i < RecHandler::UArc.GetSize(); i++) { // for each arc
		API_Element TestArc = RecHandler::UArc.Get(i);
		API_Coord BegTest = Line::GetArcBegCoord(TestArc);
		API_Coord EndTest = Line::GetArcEndCoord(TestArc);
		if (i != arcInd) {
			double sens = 0.05;
			if (Line::distanceBetweenPoints(BegTest, BegCur) < sens)
				return true;
			if (Line::distanceBetweenPoints(EndTest, EndCur) < sens)
				return true;
			if (Line::distanceBetweenPoints(EndTest, BegCur) < sens)
				return true;
			if (Line::distanceBetweenPoints(BegTest, EndCur) < sens)
				return true;
		}
	}
	return false;
}

