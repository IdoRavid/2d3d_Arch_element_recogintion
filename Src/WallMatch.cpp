#include "WallMatch.h"


GS::Array<std::pair<API_Guid, bool>> WallMatch::GetSingleWallsArr() {
	GS::Array<API_Guid>  wList;
	GS::Array<std::pair<API_Guid, bool>> SingleArr;
	ACAPI_Element_GetElemList(API_WallID, &wList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::USize i = 0; i < wList.GetSize(); i++)
	{
		std::pair<bool, bool> singleWall = IsWallSingle(wList.Get(i));
		if (singleWall.first && singleWall.second) {
			SingleArr.Push(std::make_pair(wList.Get(i), true));
			SingleArr.Push(std::make_pair(wList.Get(i), false));
		}


		else if (singleWall.first || singleWall.second)
			SingleArr.Push(std::make_pair(wList.Get(i), singleWall.first));
	}
	return SingleArr;
}

std::pair<bool, bool> WallMatch::IsWallSingle(API_Guid wall) {
	GS::Array<API_Guid> wList;
	double Sensi = 0.12;
	bool begPartner = false, endPartner = false;
	Line WallLine(wall);
	ACAPI_Element_GetElemList(API_WallID, &wList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::USize i = 0; i < wList.GetSize(); i++)
	{
		if (wall != wList.Get(i))
		{
			Line TestLine(wList.Get(i));
			if (Line::distanceBetweenPoints(WallLine.beg, TestLine.beg) < Sensi || Line::distanceBetweenPoints(WallLine.beg, TestLine.end) < Sensi || WallMatch::IsPointInWall(WallLine.beg, TestLine.guid))
				begPartner = true;
			else if (Line::distanceBetweenPoints(WallLine.end, TestLine.beg) < Sensi || Line::distanceBetweenPoints(WallLine.end, TestLine.end) < Sensi || WallMatch::IsPointInWall(WallLine.end, TestLine.guid))
				endPartner = true;
		}
		if (begPartner && endPartner) //connected at both ends
			return std::make_pair(false, false);
	}
	if (!begPartner && endPartner)
		return std::make_pair(true, false); //single at begC
	if (begPartner && !endPartner)
		return std::make_pair(false, true); //single at endC
	if (!begPartner && !endPartner)
		return std::make_pair(true, true); //unconnected - lonely wall

	//never reached
	return std::make_pair(false, false);
}


void WallMatch::ManageWallMatching()
{
	GS::Array<std::pair<API_Guid, bool>> SingleWalls = WallMatch::GetSingleWallsArr();
	if (SingleWalls.GetSize() == 0)
		return;
	GS::USize prevSize = SingleWalls.GetSize();
	GS::USize CurSize = 2;
	while (prevSize > CurSize && CurSize > 1) {
		prevSize = CurSize;
		WallMatch::MatchSingleWalls();
		CurSize = WallMatch::GetSingleWallsArr().GetSize();
	}


}

void WallMatch::MatchSingleWalls() {
	GS::Array<std::pair<API_Guid, bool>> SingleWalls = WallMatch::GetSingleWallsArr();
	if (SingleWalls.GetSize() == 0)
		return;

	//GS::Array<API_Neig>  selNeigs;
	//for (GS::USize i = 0; i < SingleWalls.GetSize(); i++)
	//{
	//	selNeigs.Push(API_Neig(SingleWalls.Get(i).first));
	//}

	//ACAPI_Element_Select(selNeigs, true);
	GS::USize size = SingleWalls.GetSize();
	for (GS::USize i = 0; i < size; ) //i++
	{
		if (SingleWalls.GetSize() < 2)
			return;
		Line CurLine = Line(SingleWalls.Get(i).first);
		API_Coord CurPoint, OtherPoint;
		if (SingleWalls.Get(i).second)
		{
			CurPoint = CurLine.beg;
			OtherPoint = CurLine.end;
		}
		else {
			CurPoint = CurLine.end;
			OtherPoint = CurLine.beg;
		}
		double distance = INFINITY;
		GS::USize ClosestInd = 0;
		for (GS::USize j = 0; j < SingleWalls.GetSize(); j++)
		{
			if (i != j && SingleWalls.Get(i).first != SingleWalls.Get(j).first)
			{

				API_Coord TestPoint;
				if (SingleWalls.Get(j).second)
					TestPoint = Line(SingleWalls.Get(j).first).beg;
				else TestPoint = Line(SingleWalls.Get(j).first).end;

				if (Line::distanceBetweenPoints(CurPoint, TestPoint) < distance)
				{
					distance = Line::distanceBetweenPoints(CurPoint, TestPoint);
					ClosestInd = j;
				}
			}
		}
		Line TestLine = Line(SingleWalls.Get(ClosestInd).first);
		API_Coord NewPoint;
		if (SingleWalls.Get(ClosestInd).second)
			NewPoint = TestLine.beg;
		else	NewPoint = TestLine.end;

		Line NewWall = Line(OtherPoint, NewPoint);
		//perpindicular or convergent


		double DistanceSens = 2;
		if ((Line::IsPerp(CurLine, TestLine) || Line::AreLinesConverging(CurLine, TestLine, 0.8)) && Line::distanceBetweenPoints(NewPoint, CurPoint) < DistanceSens && Line::GetRealLength(NewWall) >= Line::GetRealLength(CurLine)) // case 1 and 2
		{
			GS::Array<std::pair<API_Guid, bool>> InputArr;
			InputArr.Push(SingleWalls.Get(i));
			InputArr.Push(SingleWalls.Get(ClosestInd));
			if (Line::IsParallel(Line(InputArr.Get(0).first), Line(InputArr.Get(1).first)))
				WallMatch::ConnectMatchingSinglesParallel(InputArr);
			else
				WallMatch::ConnectMatchingSinglesPerpendicular(InputArr);
			SingleWalls.Delete(ClosestInd);
			SingleWalls.Delete(i);
			if (SingleWalls.GetSize() < 2)
				return;
			size = SingleWalls.GetSize();
		}

		// Else - new Wall
		else
		{
			Line NewWall = Line(NewPoint, CurPoint);
			bool slanted = abs(Line::GetAngleBetweenLines(NewWall, CurLine) - 90) < 5;
			if (Line::distanceBetweenPoints(NewWall.beg, NewWall.end) < 2 && ((Line::IsPerp(NewWall, CurLine) || slanted))) // case 4
			{
				if (!slanted) {
					double width = WallMatch::GetWallWidth(SingleWalls.Get(i).first);
					WallRec::CreateWallFromLine(Line(CurPoint, NewPoint), width);
				}
				else
				{
					double width = WallMatch::GetWallWidth(SingleWalls.Get(i).first);
					API_Coord ClosestPoint = Line::GetClosesetPointOnline(TestLine, CurPoint);
					WallRec::CreateWallFromLine(Line(CurPoint, ClosestPoint), width);

					API_Element element, mask1;
					BNZeroMemory(&element, sizeof(API_Element)); //free up memory
					BNZeroMemory(&mask1, sizeof(API_Element));
					element.header.guid = TestLine.guid; //enter the wall Guid to element1
					ACAPI_Element_Get(&element);

					if (SingleWalls.Get(ClosestInd).second)
					{
						if (Line::distanceBetweenPoints(element.wall.endC, ClosestPoint) < 0.25)
							return;
						element.wall.begC = ClosestPoint;
						ACAPI_ELEMENT_MASK_CLEAR(mask1);
						ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, begC);
					}
					else
					{
						if (Line::distanceBetweenPoints(element.wall.begC, NewPoint) < 0.25)
							return;
						element.wall.endC = ClosestPoint;
						ACAPI_ELEMENT_MASK_CLEAR(mask1);
						ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, endC);
					}
					ACAPI_CallUndoableCommand("Change Wall",
						[&]() -> GSErrCode {
							return ACAPI_Element_Change(&element, &mask1, nullptr, 1, true);
						});


				}
				SingleWalls.Delete(ClosestInd);
				SingleWalls.Delete(i);
				if (SingleWalls.GetSize() < 2)
					return;
				size = SingleWalls.GetSize();

			}
			else i++;
		}
	}
}

void WallMatch::ConnectMatchingSinglesPerpendicular(GS::Array<std::pair<API_Guid, bool>> SingleWalls)
{
	if (SingleWalls.GetSize() != 2)
		return;

	Line lineWall1 = Line(SingleWalls.Get(0).first);
	Line lineWall2 = Line(SingleWalls.Get(1).first);

	API_Coord NewPoint;
	if (SingleWalls.Get(1).second) //if New Point is line1's beg
		NewPoint = lineWall2.beg;
	else NewPoint = lineWall2.end;


	//change Wall1
	API_Element element, mask1;
	BNZeroMemory(&element, sizeof(API_Element)); //free up memory
	BNZeroMemory(&mask1, sizeof(API_Element));
	element.header.guid = SingleWalls.Get(0).first; //enter the wall Guid to element1
	ACAPI_Element_Get(&element);
	if (SingleWalls.Get(0).second && element.wall.hasDoor)
	{
		WallRec::CreateWallFromLine(Line(Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first), element.wall.begC), element.wall.thickness);
	}
	else
	{
		if (SingleWalls.Get(0).second && !element.wall.hasDoor)
		{
			if (Line::distanceBetweenPoints(element.wall.endC, NewPoint) < 0.25)
				return;
			element.wall.begC = Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first);
			if (element.wall.begC.x == INFINITY)
				return;
			ACAPI_ELEMENT_MASK_CLEAR(mask1);
			ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, begC);

		}
		else
		{
			if (Line::distanceBetweenPoints(element.wall.begC, NewPoint) < 0.25)
				return;
			element.wall.endC = Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first);;
			if (element.wall.endC.x == INFINITY)
				return;
			ACAPI_ELEMENT_MASK_CLEAR(mask1);
			ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, endC);
		}
		ACAPI_CallUndoableCommand("Change Wall",
			[&]() -> GSErrCode {
				return ACAPI_Element_Change(&element, &mask1, nullptr, 1, true);
			});
	}
	//change Wall2
	BNClear(element); //free up memory
	BNClear(mask1);
	element.header.guid = SingleWalls.Get(1).first; //enter the wall Guid to element1
	ACAPI_Element_Get(&element);


	if (SingleWalls.Get(1).second && element.wall.hasDoor)
	{
		WallRec::CreateWallFromLine(Line(Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first), element.wall.begC), element.wall.thickness);
	}
	else
	{
		if (SingleWalls.Get(1).second)
		{
			if (Line::distanceBetweenPoints(element.wall.endC, NewPoint) < 0.25)
				return;
			element.wall.begC = Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first);
			ACAPI_ELEMENT_MASK_CLEAR(mask1);
			ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, begC);
		}
		else
		{
			if (Line::distanceBetweenPoints(element.wall.begC, NewPoint) < 0.25)
				return;
			element.wall.endC = Line::LineMeetingPoint(SingleWalls.Get(1).first, SingleWalls.Get(0).first);;
			ACAPI_ELEMENT_MASK_CLEAR(mask1);
			ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, endC);
		}
		ACAPI_CallUndoableCommand("Change Wall",
			[&]() -> GSErrCode {
				return ACAPI_Element_Change(&element, &mask1, nullptr, 1, true);
			});
	}

}

void WallMatch::ConnectMatchingSinglesParallel(GS::Array<std::pair<API_Guid, bool>> SingleWalls)
{
	if (SingleWalls.GetSize() != 2)
		return;

	Line lineWall1 = Line(SingleWalls.Get(0).first);
	Line lineWall2 = Line(SingleWalls.Get(1).first);

	API_Coord NewPoint;
	if (SingleWalls.Get(1).second) //if New Point is line1's beg
		NewPoint = lineWall2.beg;
	else NewPoint = lineWall2.end;


	//change Wall1

	API_Element element, mask1;
	BNZeroMemory(&element, sizeof(API_Element)); //free up memory
	BNZeroMemory(&mask1, sizeof(API_Element));
	element.header.guid = SingleWalls.Get(0).first; //enter the wall Guid to element1
	ACAPI_Element_Get(&element);

	int i = 0;
	int j = 1;
	if (element.wall.hasDoor)
	{
		i = 1;
		j = 0;
		BNClear(element); //free up memory
		BNClear(mask1);
		element.header.guid = SingleWalls.Get(1).first; //enter the wall Guid to element1
		ACAPI_Element_Get(&element);
	}



	if (SingleWalls.Get(i).second)
	{
		if (Line::distanceBetweenPoints(element.wall.endC, NewPoint) < 0.25)
			return;
		if (SingleWalls.Get(j).second)
			element.wall.begC = lineWall2.beg;
		else element.wall.begC = lineWall2.end;
		ACAPI_ELEMENT_MASK_CLEAR(mask1);
		ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, begC);
	}
	else
	{
		if (Line::distanceBetweenPoints(element.wall.begC, NewPoint) < 0.25)
			return;
		if (SingleWalls.Get(j).second)
			element.wall.endC = lineWall2.beg;
		else element.wall.endC = lineWall2.end;
		ACAPI_ELEMENT_MASK_CLEAR(mask1);
		ACAPI_ELEMENT_MASK_SET(mask1, API_WallType, endC);
	}
	ACAPI_CallUndoableCommand("Change Wall",
		[&]() -> GSErrCode {
			return ACAPI_Element_Change(&element, &mask1, nullptr, 1, true);
		});
}

double WallMatch::GetWallWidth(API_Guid wall)
{
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element)); //free up memory
	element.header.guid = wall; //enter the wall Guid to element1
	ACAPI_Element_Get(&element);

	return element.wall.thickness;
}

bool WallMatch::IsPointInWall(API_Coord point, API_Guid wall)
{
	double Sens = 0.05;
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = wall;
	ACAPI_Element_Get(&element);

	API_Box3D box3D;
	BNZeroMemory(&box3D, sizeof(API_Box3D));
	GS::ErrCode err = ACAPI_Database(APIDb_CalcBoundsID, &element.header, &box3D);
	if (err != NoError)
		return false;

	if (point.x >= (box3D.xMin - Sens) && point.x <= (box3D.xMax + Sens) && point.y >= (box3D.yMin - Sens) && point.y <= (box3D.yMax + Sens))
		return true;
	return false;
}

void WallMatch::SelectSingleWalls()
{
	GS::Array<std::pair<API_Guid, bool>> SingleWalls = WallMatch::GetSingleWallsArr();
	TmrTech::Do_DeselectAll();
	GS::Array<API_Neig>  selNeigs;
	for (GS::USize i = 0; i < SingleWalls.GetSize(); i++)
	{
		selNeigs.Push(API_Neig(SingleWalls.Get(i).first));
	}
	ACAPI_Element_Select(selNeigs, true);
	ACAPI_Database(APIDb_ResetCurrentWindowID);

}