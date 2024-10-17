#include "WallRec.h"
#include <array>


short WallRec::WallPenInd = 0;




GS::Array<Line> WallRec::GetLineArr()
{
	return RecHandler::UL;
}

GS::Array < std::pair < Line, double >> WallRec::GetWallLinePairs() {

	GS::Array < std::pair < Line, double >>  MidLineArr;
	for (GS::USize i = 0; i < RecHandler::UL.GetSize(); )
	{
		GS::Array <GS::USize> TestLinesInd;
		Line CurLine = RecHandler::UL.Get(i);
		for (GS::USize j = 0; j < RecHandler::UL.GetSize(); j++)
		{
			Line TestLine = RecHandler::UL.Get(j);
			if (i != j && Line::IsParallel(CurLine, TestLine)) //if lines are parrallel
			{
				if (RecHandler::layerMap[GS::UniString("Wall")].GetSize() == 0 || (RecHandler::layerMap[GS::UniString("Wall")].Contains(CurLine.layer) && RecHandler::layerMap[GS::UniString("Wall")].Contains(TestLine.layer) || (1 == CurLine.layer && 1 == TestLine.layer))) {
					if (Line::distanceBetweenLines(CurLine, TestLine) < 0.25 && Line::distanceBetweenLines(CurLine, TestLine) > 0.01) //if line distance is smaller than 0.2;
						if (ParrallelUnconnectedLineTest(CurLine, TestLine)) { //if lines only have one shared point, i.e windows
							if (WallRec::ReturnLinePenInd(CurLine.guid) == WallRec::ReturnLinePenInd(TestLine.guid)) { // if their pens are similar
								TestLinesInd.Push(j);
							}
						}
				}
			}
		}

		if (TestLinesInd.GetSize() == 1) {
			if (Line::IsParallel(RecHandler::UL.Get(TestLinesInd.Get(0)), CurLine) && CurLine.guid != RecHandler::UL.Get(TestLinesInd.Get(0)).guid) {
				MidLineArr.Push(std::make_pair(WallRec::GetMidLine(CurLine, RecHandler::UL.Get(TestLinesInd.Get(0)), false), Line::distanceBetweenLines(CurLine, RecHandler::UL.Get(TestLinesInd.Get(0))))); // push the middle line between;
				RecHandler::UL.Delete(TestLinesInd.Get(0)); //del from Unidentified
				RecHandler::UL.Delete(i); //del from Unidentified
			}
		}

		//if (TestLinesInd.GetSize() > 1) {
		//	GS::USize numDeleted = 0;
		//	//if more than one line supposedly fits - we want to find the line with most convergents. get that line and delete every other line
		//	TestLinesInd= WallRec::FilterRelevantWallLines(TestLinesInd);
		//	while ( TestLinesInd.GetSize() > 0)
		//	{
		//		MidLineArr.Push(std::make_pair(WallRec::GetMidLine(CurLine, UL.Get(TestLinesInd.Get(0) - numDeleted), true), Line::distanceBetweenLines(CurLine, UL.Get(TestLinesInd.Get(0) - numDeleted)))); // push the middle line between;
		//		UL.Delete(TestLinesInd.Get(0)); //del from Unidentified
		//		TestLinesInd.Delete(0);
		//		numDeleted++;
		//	}
		//	UL.Delete(i); //del from Unidentified == curLine


		//}


		else i++;
	}


	MidLineArr = WallRec::RemoveDupLineInd(MidLineArr, true, false);
	return MidLineArr;

}

API_Guid WallRec::CreateWallFromLine(Line line, double width) {
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	element.header.typeID = API_WallID;
	element.header.variationID = APIVarId_Object;
	GSErrCode err = ACAPI_Element_GetDefaults(&element, &memo);
	element.wall.referenceLineLocation = APIWallRefLine_Center;
	element.wall.begC = line.beg;
	element.wall.endC = line.end;
	element.wall.thickness = width;
	element.wall.thickness1 = width;
	err = ACAPI_CallUndoableCommand("Create Wall",
		[&]() -> GSErrCode {
			return err = ACAPI_Element_Create(&element, &memo); });
	if (err != NoError) {
		ACAPI_WriteReport("Couldn't Create Wall", true, err);
	}
	return element.header.guid;
}


void WallRec::CreateWallsFromDrawingLines() {

	GS::Array < std::pair < Line, double >> MidLines = WallRec::GetWallLinePairs();
	for (GS::USize i = 0; i < MidLines.GetSize(); i++)
	{
		WallRec::CreateWallFromLine(MidLines.Get(i).first, MidLines.Get(i).second);
	}
}

Line WallRec::GetMidLine(Line lineA, Line lineB, bool MultiLine) {
	if (!Line::IsParallel(lineA, lineB))
		return Line();
	if (!MultiLine) {
		if (lineA.ParallelToX && lineB.ParallelToX) {
			API_Coord c;
			BNZeroMemory(&c, sizeof(API_Coord));
			c.y = (lineA.beg.y + lineB.beg.y) / 2;
			c.x = std::min(lineA.beg.x, lineB.beg.x);
			API_Coord d;
			BNZeroMemory(&d, sizeof(API_Coord));
			d.y = (lineA.end.y + lineB.end.y) / 2;
			d.x = std::max(lineA.end.x, lineB.end.x);
			Line ret = Line(c, d, lineA, lineB);
			ret.PenInd = lineA.PenInd;
			return ret;
		}

		if (lineA.ParallelToY && lineB.ParallelToY) {
			API_Coord c;
			c.x = (lineA.beg.x + lineB.beg.x) / 2;
			c.y = std::min(lineA.beg.y, lineB.beg.y);
			API_Coord d;
			BNZeroMemory(&d, sizeof(API_Coord));
			d.x = (lineA.beg.x + lineB.beg.x) / 2;
			d.y = std::max(lineA.end.y, lineB.end.y);
			Line ret = Line(c, d, lineA, lineB);
			ret.PenInd = lineA.PenInd;
			return ret;

		}

		API_Coord c, d;

		c.x = (lineA.beg.x + lineB.beg.x) / 2;
		c.y = (lineA.beg.y + lineB.beg.y) / 2;

		d.x = (lineA.end.x + lineB.end.x) / 2;
		d.y = (lineA.end.y + lineB.end.y) / 2;
		Line ret = Line(c, d, lineA, lineB);
		ret.PenInd = lineA.PenInd;
		return ret;
	}
	else {
		Line Smaller;
		if (Line::GetRealLength(lineA) > Line::GetRealLength(lineB))
			 Smaller  =  Line(lineB);
		else  Smaller = lineA;
		if (lineA.ParallelToX && lineB.ParallelToX) {
			API_Coord c;
			BNZeroMemory(&c, sizeof(API_Coord));
			c.y = (lineA.beg.y + lineB.beg.y) / 2;
			c.x = std::min(Smaller.beg.x, Smaller.end.x);
			API_Coord d;
			BNZeroMemory(&d, sizeof(API_Coord));
			d.y = (lineA.end.y + lineB.end.y) / 2;
			d.x = std::max(Smaller.beg.x, Smaller.end.x);
			Line ret = Line(c, d, lineA, lineB);
			ret.PenInd = lineA.PenInd;
			return ret;
		}

		if (lineA.ParallelToY && lineB.ParallelToY) {
			API_Coord c;
			c.x = (lineA.beg.x + lineB.beg.x) / 2;
			c.y = std::min(Smaller.end.y, Smaller.beg.y);
			API_Coord d;
			BNZeroMemory(&d, sizeof(API_Coord));
			d.x = (lineA.beg.x + lineB.beg.x) / 2;
			d.y = std::max(Smaller.end.y, Smaller.beg.y);
			Line ret = Line(c, d, lineA, lineB);
			ret.PenInd = lineA.PenInd;
			return ret;

		}

		API_Coord c, d;

		c.x = (lineA.beg.x + lineB.beg.x) / 2;
		c.y = (lineA.beg.y + lineB.beg.y) / 2;

		d.x = (lineA.end.x + lineB.end.x) / 2;
		d.y = (lineA.end.y + lineB.end.y) / 2;
		Line ret = Line(c, d, lineA, lineB);
		ret.PenInd = lineA.PenInd;
		return ret;






	}
}

bool WallRec::ParrallelUnconnectedLineTest(Line lineA, Line lineB) { // if false - not fit to be in wall together
	if (!Line::IsParallel(lineA, lineB))
		return false;
	if (lineA.ParallelToX && lineB.ParallelToX) {
		double maxXA = std::max(lineA.beg.x, lineA.end.x);
		double minXA = std::min(lineA.beg.x, lineA.end.x);
		double maxXB = std::max(lineB.beg.x, lineB.end.x);
		double minXB = std::min(lineB.beg.x, lineB.end.x);
		double minX = std::min(minXA, minXB);
		double maxX = std::max(maxXA, maxXB);

		double SumX = abs(minX - maxX);
		double SumLength = Line::GetRealLength(lineA) + Line::GetRealLength(lineB);
		if (SumLength > SumX)
			return true;
		else return false;
	}
	if (lineA.ParallelToY && lineB.ParallelToY) {
		double maxYA = std::max(lineA.beg.y, lineA.end.y);
		double minYA = std::min(lineA.beg.y, lineA.end.y);
		double maxYB = std::max(lineB.beg.y, lineB.end.y);
		double minYB = std::min(lineB.beg.y, lineB.end.y);
		double minY = std::min(minYA, minYB);
		double maxY = std::max(maxYA, maxYB);

		double SumY = abs(minY - maxY);
		double SumLength = Line::GetRealLength(lineA) + Line::GetRealLength(lineB);
		if (SumLength > SumY)
			return true;
		else return false;
	}
	double y1 = lineA.beg.y;
	double y2 = lineA.end.y;
	double y3 = lineB.beg.y;
	double y4 = lineB.end.y;

	double arr[] = { y1, y2, y3, y4 };
	std::sort(arr, arr + 4);
	double MidDist = abs(arr[1] - arr[2]);
	if (MidDist > 0.01)
		return true;
	return false;
}


short WallRec::ReturnLinePenInd(API_Guid g) {
	API_Element line;
	BNZeroMemory(&line, sizeof(API_Element));
	line.header.guid = g;
	ACAPI_Element_Get(&line);

	return line.line.linePen.penIndex;
}


GS::Array < std::pair < Line, double> > WallRec::RemoveDupLineInd(GS::Array < std::pair < Line, double> > lArr, bool isWall, bool isWindow) {
	std::map<short, int> map;
	short penVal = 0;
	int penCount = 0;

	for (GS::USize i = 0; i < lArr.GetSize();)
	{
		if (lArr.Get(i).first.PenInd == 0) {
			RecHandler::UL.Push(Line(lArr.Get(i).first.ParentLines.first));
			RecHandler::UL.Push(Line(lArr.Get(i).first.ParentLines.second));
			lArr.Delete(i);
		}
		else i++;
	}

	for (GS::USize i = 0; i < lArr.GetSize(); i++)
	{
		std::map<short, int>::iterator it = map.find(lArr.Get(i).first.PenInd);
		if (it != map.end())
		{
			map[lArr.Get(i).first.PenInd] = map[lArr.Get(i).first.PenInd]++;
		}
		else {
			map[lArr.Get(i).first.PenInd] = 1;
		}
	}
	//count num of each pen

	for (std::map<short, int>::iterator ita = map.begin(); ita != map.end(); ita++)
	{
		if (penCount < ita->second)
		{
			penCount = ita->second;
			penVal = ita->first;
		}

	}

	for (GS::USize i = 0; i < lArr.GetSize();)
	{
		short p = lArr.Get(i).first.PenInd;
		if (p != penVal) {
			RecHandler::UL.Push(Line(lArr.Get(i).first.ParentLines.first));
			RecHandler::UL.Push(Line(lArr.Get(i).first.ParentLines.second));
			lArr.Delete(i);
		}
		else i++;
	}
	if (isWall)
		WallRec::WallPenInd = penVal;
	if (isWindow)
		WindowRec::WindPenInd = penVal;
	return lArr;
}



void WallRec::DeleteSuperFarWalls() {
	GS::Array<API_Guid> wList;
	ACAPI_Element_GetElemList(API_WallID, &wList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::Array<API_Guid>::ConstIterator it = wList.Enumerate(); it != nullptr; ++it)
	{ // for each WALL
		API_Element wall;
		BNZeroMemory(&wall, sizeof(API_Element));
		wall.header.guid = *it;
		ACAPI_Element_Get(&wall);
		API_Coord zero;
		zero.x = 0, zero.y = 0;
		if (Line::distanceBetweenPoints(zero, wall.wall.begC) > 600) {
			GS::Array<API_Guid> elemHeads;
			elemHeads.Push(*it);
			ACAPI_CallUndoableCommand("Delete Far Wall",
				[&]() -> GSErrCode {
					return ACAPI_Element_Delete(elemHeads);
				});

		}
	}


}


/*
//int WallRec::GetClosestPointCombination(Line lineA, Line lineB) {
//	double dist = INFINITY;
//	int cas1 = 0;
//	if (dist > Line::distanceBetweenPoints(lineA.beg, lineB.beg))
//	{
//		dist = Line::distanceBetweenPoints(lineA.beg, lineB.beg);
//		cas1 = 1;
//	}
//	if (dist > Line::distanceBetweenPoints(lineA.beg, lineB.end))
//	{
//		dist = Line::distanceBetweenPoints(lineA.beg, lineB.end);
//		cas1 = 2;
//	}
//	if (dist > Line::distanceBetweenPoints(lineA.end, lineB.beg))
//	{
//		dist = Line::distanceBetweenPoints(lineA.end, lineB.beg);
//		cas1 = 3;
//	}
//	if (dist > Line::distanceBetweenPoints(lineA.end, lineB.end))
//	{
//		dist = Line::distanceBetweenPoints(lineA.end, lineB.end);
//		cas1 = 4;
//	}
//
//	return cas1;
//
//}
//bool WallRec::SplitWallTest(Line lineA, Line lineB, Line lineC) {
//
//	if (!Line::AreLinesConverging(lineA, lineB))
//		return false;
//
//	API_Coord a, b;
//	a.y = INFINITY, a.y = INFINITY;
//	b = a;
//	int c = WallRec::GetClosestPointCombination(lineA, lineB);
//	switch (c) {
//
//	case 0:
//		return false; break;
//	default:
//		return false; break;
//
//	case 1:
//		a = lineA.beg;
//		b = lineB.beg;
//		break;
//
//	case 2:
//		a = lineA.beg;
//		b = lineB.end;
//		break;
//
//	case 3:
//		a = lineA.end;
//		b = lineB.beg;
//		break;
//
//	case 4:
//		a = lineA.end;
//		b = lineB.end;
//		break;
//	}
//	if (lineA.ParallelToX && lineB.ParallelToX) {
//		if (a.y > std::min(lineC.beg.x, lineC.end.x) && a.y < std::max(lineC.beg.x, lineC.end.x))
//			if (b.y > std::min(lineC.beg.x, lineC.end.x) && b.y < std::max(lineC.beg.x, lineC.end.x))
//				return true;
//			else return false;
//		else return false;
//	}
//	else {
//		if (a.y > std::min(lineC.beg.y, lineC.end.x) && a.y < std::max(lineC.beg.y, lineC.end.x))
//			if (b.y > std::min(lineC.beg.y, lineC.end.x) && b.y < std::max(lineC.beg.y, lineC.end.x))
//				return true;
//			else return false;
//		else return false;
//	}
//}
/*
void WallRec::CreateWallsFromIntersectingLines() {
	GS::Array<API_Guid> WallList;
	bool created = false;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan);
	GS::USize Si = WallList.GetSize();
	for (GS::USize j = 0; j < Si; j++) {
		GS::Array < std::pair<GS::USize, Line>> PotentialWalls;
		for (GS::USize i = 0; i < WallRec::UL.GetSize(); i++)
		{
			if (WallRec::ReturnLinePenInd(WallRec::UL.Get(i).guid) == WallPenInd)
			{
				if (WallRec::IsWallLineIntersection(WallRec::UL.Get(i), WallList.Get(j))) {
					//if intersection of some type
					PotentialWalls.Push(std::make_pair(i, UL.Get(i)));
					break;
				}
			}
		}

		for (GS::USize k = 0; k < PotentialWalls.GetSize(); k++)
		{
			//tryCreatingLinePairs
			for (GS::USize s = 0; s < PotentialWalls.GetSize();) {

				if (k != s) {
					if (Line::distanceBetweenLines(PotentialWalls.Get(k).second, PotentialWalls.Get(s).second) < 0.25 && Line::distanceBetweenLines(PotentialWalls.Get(k).second, PotentialWalls.Get(s).second) > 0.01) { //if line distance is smaller than 0.2;
						if (ParrallelUnconnectedLineTest(PotentialWalls.Get(k).second, PotentialWalls.Get(s).second)) { //if lines only have one shared point, i.e windows


							double width = Line::distanceBetweenLines(PotentialWalls.Get(k).second, PotentialWalls.Get(s).second);
							WallRec::CreateWallFromLine(WallRec::GetMidLine(PotentialWalls.Get(k).second, PotentialWalls.Get(s).second), width);
							created = true;
							//deleteLinesFromPotential
							WallRec::WallLines.Push(UL.Get(PotentialWalls.Get(s).first));
							WallRec::WallLines.Push(UL.Get(PotentialWalls.Get(k).first));
							UL.Delete(PotentialWalls.Get(s).first);
							UL.Delete(PotentialWalls.Get(k).first);

							//PotentialWalls.Delete(s);
							//PotentialWalls.Delete(k);


						}
						else s++;
					}
					else s++;
				}
				else s++;
			}

			k++;
		}




		if (created)
		{
			WallList.Clear();
			ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan);
			Si = WallList.GetSize();
			if (j + 2 == Si)
				break;
		}
		j++;



	}
}


bool WallRec::IsWallLineIntersection(Line line, API_Guid WallGuid) {

	line = line;
	WallGuid = WallGuid;
	return true;
}

void WallRec::TryCreatingWallFromUnidentified() {
	GS::Array<Line> CreatedArr;
	for (GS::USize i = 0; i < WallRec::UL.GetSize(); )

	{
		bool created = false;
		for (GS::USize j = 0; j < WallRec::UL.GetSize(); j++)
		{
			Line CurLine = WallRec::UL.Get(i);
			Line TestLine = WallRec::UL.Get(j);
			if (CurLine.PenInd == WallPenInd && i != j) {
				if (TestLine.PenInd == WallPenInd) {
					if (Line::distanceBetweenLines(TestLine, CurLine) < 0.25) {
						if (WallRec::GetClosestPointDistance(TestLine, CurLine) < 0.5) {
							(WallRec::CreateWallFromLine(WallRec::GetMidLine(CurLine, TestLine), Line::distanceBetweenLines(CurLine, TestLine)));
							created = true;
							CreatedArr.Push(TestLine);
							CreatedArr.Push(CurLine);
							WallRec::UL.Delete(i);
						}
					}
				}
			}

		}
		if (!created)
			i++;
	}
	CreatedArr = WallRec::DeleteDupLines(CreatedArr);
	for (GS::USize i = 0; i < CreatedArr.GetSize(); i++)
	{
		WallRec::WallLines.Push(CreatedArr.Get(i));
	}

}

GS::Array<Line> WallRec::DeleteDupLines(GS::Array<Line> lineArr) {
	for (GS::USize i = 0; i < lineArr.GetSize(); i++)
	{
		for (GS::USize j = 0; j < lineArr.GetSize(); )
		{
			if (lineArr.Get(i).beg.x == lineArr.Get(j).beg.x && lineArr.Get(i).end.x == lineArr.Get(j).end.x) {
				if (lineArr.Get(i).beg.y == lineArr.Get(j).beg.y && lineArr.Get(i).end.y == lineArr.Get(j).end.y) {
					if (i != j) {
						lineArr.Delete(j);
					}
					else j++;
				}
				else j++;
			}
			else j++;
		}

	}
	return lineArr;

}

double WallRec::GetClosestPointDistance(Line lineA, Line lineB) {
	API_Coord a, b;
	a.y = INFINITY, a.y = INFINITY;
	b = a;
	int c = WallRec::GetClosestPointCombination(lineA, lineB);
	switch (c) {

	case 0:
		return 0; break;
	default:
		return 0; break;

	case 1:
		a = lineA.beg;
		b = lineB.beg;
		break;

	case 2:
		a = lineA.beg;
		b = lineB.end;
		break;

	case 3:
		a = lineA.end;
		b = lineB.beg;
		break;

	case 4:
		a = lineA.end;
		b = lineB.end;
		break;
	}
	return Line::distanceBetweenPoints(a, b);
}

*/