#include "WindowRec.h"
#include "Numberer.h"
GS::Array<Line> WindowRec::WindLines;
GS::Array<API_Element> WindowRec::WindArcs;
short WindowRec::WindPenInd = 0;



void WindowRec::CreateWindowsFromDrawingLines()
{
	//iterate between Unidentified Lines
	// find a solid window pattern - delete converging dups
	//create a Window Line and add it to window line array - decrement from UL
	GS::Array <Line>	WindMidLinePairs = GetWindLinePairs();

	//for each window in line array - check if is nearby a wall
	//if so - change the wall so it takes the space of the window
	WindowRec::CreateWallInWindLine(WindMidLinePairs);

	//instantiate a window in the relevant location using the line from the array
	//select the relevant sizes and heights of the window based on its width
	for (GS::USize i = 0; i < WindMidLinePairs.GetSize(); i++)
	{
		if (Line::GetRealLength(WindMidLinePairs.Get(i)) < 10)
			WindowRec::CreatWindowFromLine(WindMidLinePairs.Get(i));
	}

}


GS::Array <Line> WindowRec::GetWindLinePairs() {

	GS::Array <  Line>  MidLineArr;
	for (GS::USize i = 0; i < RecHandler::UL.GetSize(); )
	{
		GS::Array <GS::USize> TestLinesInd;
		Line CurLine = RecHandler::UL.Get(i);
		for (GS::USize j = 0; j < RecHandler::UL.GetSize(); j++)
		{
			Line TestLine = RecHandler::UL.Get(j);
			if (i != j && Line::IsParallel(CurLine, TestLine)) //if lines are parrallel
			{
				if (Line::distanceBetweenLines(CurLine, TestLine) < 0.25 && Line::distanceBetweenLines(CurLine, TestLine) > 0.01) //if line distance is smaller than 0.2;
					if (WallRec::ParrallelUnconnectedLineTest(CurLine, TestLine)) { //if lines only have one shared point, i.e windows
						if (WallRec::ReturnLinePenInd(CurLine.guid) == WallRec::ReturnLinePenInd(TestLine.guid)) // if their pens are similar
						{
							if ((RecHandler::layerMap[GS::UniString("Window")].GetSize() == 0) || (RecHandler::layerMap[GS::UniString("Window")].Contains(CurLine.layer) && RecHandler::layerMap[GS::UniString("Window")].Contains(TestLine.layer) || (1 == CurLine.layer && 1 == TestLine.layer)))
							{
								TestLinesInd.Push(j);
							}
						}
					}
			}



		}


		if (TestLinesInd.GetSize() > 1) {

			//we want to make sure there are at least 3 lines parralell to each other, without convergents;

			if (TestLinesInd.GetSize() > 4)
				TestLinesInd = WindowRec::FilterConvergentWindLines(TestLinesInd);
			if (TestLinesInd.GetSize() > 1)
			{
				Line WindMidLineAndWidth = WindowRec::GetWindowMidLine(TestLinesInd);
				MidLineArr.Push(WindMidLineAndWidth);
				GS::USize NumDel = 0;
				for (GS::USize k = 0; k < TestLinesInd.GetSize(); k++) {
					RecHandler::UL.Delete(TestLinesInd.Get(k) + NumDel); //del from Unidentified
					NumDel++;
				}
			}
			RecHandler::UL.Delete(i); //del from Unidentified == curLine
		}
		else i++;
	}
	MidLineArr = WindowRec::RemoveDupLineInd(MidLineArr, false, true);
	return MidLineArr;
}
GS::Array<GS::USize>  WindowRec::FilterConvergentWindLines(GS::Array<GS::USize> IndArr) {
	for (GS::USize i = 0; i < IndArr.GetSize(); i++)
	{
		Line CurLine = RecHandler::UL.Get(i);
		for (GS::USize j = 0; j < IndArr.GetSize(); )
		{
			if (i != j && Line::AreLinesConverging(RecHandler::UL.Get(IndArr.Get(i)), RecHandler::UL.Get(IndArr.Get(j)), 0.5))
				IndArr.Delete(j);
			else j++;


		} //rank each line by num of Convergents

	}
	return IndArr;
}

Line  WindowRec::GetWindowMidLine(GS::Array <GS::USize> TestLinesInd) {


	//first, get outermost lines
	GS::USize maxInd = 0;
	GS::USize minInd = 0;
	if (RecHandler::UL.Get(TestLinesInd.Get(0)).ParallelToX) {
		//if line arr is parr to x
		double MinY = INFINITY;
		double MaxY = -INFINITY;
		for (GS::USize i = 0; i < TestLinesInd.GetSize(); i++)
		{
			if (MinY > RecHandler::UL.Get(TestLinesInd.Get(i)).beg.y)
			{
				MinY = RecHandler::UL.Get(TestLinesInd.Get(i)).beg.y;
				minInd = i;
			}
			if (MaxY < RecHandler::UL.Get(TestLinesInd.Get(i)).beg.y)
			{
				MaxY = RecHandler::UL.Get(TestLinesInd.Get(i)).beg.y;
				maxInd = i;
			}
		}

	}
	else {
		if (RecHandler::UL.Get(TestLinesInd.Get(0)).ParallelToY) {
			//if line arr is parr to Y
			double MinX = INFINITY;
			double MaxX = -INFINITY;
			for (GS::USize i = 0; i < TestLinesInd.GetSize(); i++)
			{
				if (MinX > RecHandler::UL.Get(TestLinesInd.Get(i)).beg.x)
				{
					MinX = RecHandler::UL.Get(TestLinesInd.Get(i)).beg.x;
					minInd = i;
				}
				if (MaxX < RecHandler::UL.Get(TestLinesInd.Get(i)).beg.x)
				{
					MaxX = RecHandler::UL.Get(TestLinesInd.Get(i)).beg.x;
					maxInd = i;
				}
			}

		}
		else {
			//if line arr isn't parr to anything
			double MinC = INFINITY;
			double MaxC = -INFINITY;
			for (GS::USize i = 0; i < TestLinesInd.GetSize(); i++)
			{
				if (MinC > RecHandler::UL.Get(TestLinesInd.Get(i)).c)
				{
					MinC = RecHandler::UL.Get(TestLinesInd.Get(i)).c;
					minInd = i;
				}
				if (MaxC < RecHandler::UL.Get(TestLinesInd.Get(i)).c)
				{
					MaxC = RecHandler::UL.Get(TestLinesInd.Get(i)).c;
					maxInd = i;
				}
			}

		}
	}
	Line line = WallRec::GetMidLine(RecHandler::UL.Get(TestLinesInd.Get(minInd)), RecHandler::UL.Get(TestLinesInd.Get(maxInd)), false);
	return line;
}

void WindowRec::CreateWallInWindLine(GS::Array < Line> WindMidLinePairs) {
	for (GS::USize i = 0; i < WindMidLinePairs.GetSize(); i++)
	{
		Line CurLine = WindMidLinePairs.Get(i);
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
			if (element.wall.angle == 0) {
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

}


GS::Array < Line> WindowRec::RemoveDupLineInd(GS::Array < Line> lArr, bool isWall, bool isWindow) {
	std::map<short, int> map;
	short penVal = 0;
	int penCount = 0;

	for (GS::USize i = 0; i < lArr.GetSize();)
	{
		if (lArr.Get(i).PenInd < 0) {
			lArr.Delete(i);
		}
		else i++;
	}

	for (GS::USize i = 0; i < lArr.GetSize(); i++)
	{
		std::map<short, int>::iterator it = map.find(lArr.Get(i).PenInd);
		if (it != map.end())
		{
			map[lArr.Get(i).PenInd] = map[lArr.Get(i).PenInd]++;
		}
		else {
			map[lArr.Get(i).PenInd] = 1;
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
		short p = lArr.Get(i).PenInd;
		if (p != penVal)
			lArr.Delete(i);
		else i++;
	}
	if (isWall)
		WallRec::WallPenInd = penVal;
	if (isWindow)
		WindowRec::WindPenInd = penVal;
	return lArr;
}

void WindowRec::CreatWindowFromLine(Line line) {
	API_Element			element;
	API_Guid wallGuid = APINULLGuid;
	API_Coord BegC;
	API_ElementMemo		memo;
	API_SubElemMemoMask	marker;
	GSErrCode			err = NoError;
	API_AddParType** markAddPars;
	double windWidth = Line::GetRealLength(line);
	API_Coord WindCenter;
	WindCenter.x = (line.beg.x + line.end.x) / 2;
	WindCenter.y = (line.beg.y + line.end.y) / 2;

	GS::Array<API_Guid> WallList;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	for (GS::USize j = 0; j < WallList.GetSize(); j++)
	{
		element;
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = WallList.Get(j);
		ACAPI_Element_Get(&element);

		if (Line::IsPointOnRealPartOfLine(WindCenter, element.wall.begC, element.wall.endC))
		{
			BegC = element.wall.begC;
			wallGuid = element.header.guid;
			break;
		}
	}

	if (wallGuid == APINULLGuid) {
		if (Line::GetRealLength(line) < 0.25)
			return;
		wallGuid = WallRec::CreateWallFromLine(line, 0.2);
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = wallGuid;
		ACAPI_Element_Get(&element);
		BegC = element.wall.begC;
	}


	element.header.typeID = API_WindowID;
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
	libPart.index = 108;
	double a, b;
	Int32 addParNum;
	BNZeroMemory(&libPart, sizeof(libPart));
	GS::ucscpy(libPart.docu_UName, L("Window 23"));
	if (ACAPI_LibPart_Search(&libPart, false) != NoError) // find door 21 in the libraries,
	{
		ACAPI_WriteReport("Error in Finding that library part: ", false);
	}
	ACAPI_LibPart_Get(&libPart);
	err = ACAPI_LibPart_GetParams(libPart.index, &a, &b, &addParNum, &markAddPars);

	marker.memo.params = markAddPars;
	marker.subElem.object.pen = 166;
	marker.subElem.object.useObjPens = true;
	element.window.objLoc = DistCPtr(&WindCenter, &BegC);
	element.window.owner = wallGuid;

	if (windWidth > 0.8)
	{
		element.window.lower = 2.1 - (windWidth * 0.75); //HEIGHT	from floor
	}
	else {

		element.window.lower = 1.8; //HEIGHT	from floor
	}
	err = ACAPI_CallUndoableCommand("Create Window",
		[&]() -> GSErrCode {
			return 	err = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
		});
	if (err != NoError)
		ErrorBeep("ACAPI_Element_CreateExt", err);



	ACAPI_DisposeElemMemoHdls(&memo);
	ACAPI_DisposeElemMemoHdls(&marker.memo);

	Numberer::ChangeParamValue(element, "A", windWidth); //WIDTH
	if (windWidth > 0.8)
	{
		Numberer::ChangeParamValue(element, "B", windWidth * 0.75); //HEIGHT from floor
	}
	else {
		Numberer::ChangeParamValue(element, "B", windWidth); //HEIGHT	
	}
	return;
}



