#include "CurWallRec.h"

void CurWallRec::CurvedWallsCreate() {
	CurWallRec::CreateCurvedWallsFromIndependentDrawing();
}
GS::Array<GS::USize> CurWallRec::FilterRelevantWallLines(GS::Array<GS::USize> IndArr) {
	std::map<GS::USize, int> map;
	GS::Array<GS::USize> empty;
	for (GS::USize i = 0; i < IndArr.GetSize(); i++)
	{
		map[IndArr.Get(i)] = 0;
		Line CurLine = RecHandler::UL.Get(i);
		for (GS::USize j = 0; j < IndArr.GetSize(); j++)
		{
			if (i != j) {
				if (Line::AreLinesConverging(RecHandler::UL.Get(IndArr.Get(i)), RecHandler::UL.Get(IndArr.Get(j)), 0.05))

					map[IndArr.Get(i)] = map[IndArr.Get(i)] + 1;
			}
		}
	} //rank each line by num of Convergents

	int HighestVal = 0;
	for (std::map<GS::USize, int>::iterator ita = map.begin(); ita != map.end(); ita++)
	{
		if (HighestVal < ita->second)
		{
			HighestVal = ita->second;
		}

	} //get Higest Ranking Value
	GS::Array<GS::USize> ToDel;
	for (std::map<GS::USize, int>::iterator ita = map.begin(); ita != map.end(); ita++)
	{
		if (HighestVal > ita->second)
		{
			ToDel.Push(ita->first);
		}
	}

	if (HighestVal == 0)
		return empty;
	GS::USize NumDeleted = 0;
	while (IndArr.GetSize() > GS::USize(HighestVal + 1))
	{
		IndArr.Delete(ToDel.Get(0) - NumDeleted);
		NumDeleted++;
	}
	return IndArr;
}

void		CurWallRec::Do_CreateCurvedWall(double  angle, API_Coord begC, API_Coord endC, double width)
{

	API_Element			element = {}, mask = {};
	GSErrCode			err;

	element.header.typeID = API_WallID;
	ACAPI_Element_GetDefaults(&element, nullptr);

	// input the arc
	element.wall.begC = begC;
	element.wall.endC = endC;

	element.wall.thickness = width;
	element.wall.thickness1 = width;
	element.wall.angle = angle;
	element.wall.referenceLineLocation = APIWallRefLine_Inside;

	element.wall.relativeTopStory = 0;
	element.wall.height = 3.0;

	err = ACAPI_CallUndoableCommand("Create curved wall",
		[&]() -> GSErrCode { return ACAPI_Element_Create(&element, nullptr); });
	if (err != NoError) {
		ErrorBeep("ACAPI_Element_Create (curved wall)", err);
		return;
	}

}		// Do_CreateCurvedWall




void CurWallRec::CreateCurvedWallsFromNearbyDrawing() {
	GS::Array <std::pair< GS::USize, std::pair<API_Element, std::pair<API_Guid, API_Guid>>>> WallArcs;
	GS::Array<std::pair<API_Element, API_Element>> arcPairs;
	GS::Array<GS::USize> WallIndArr;
	GS::Array<API_Guid> WallList;
	ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
	GS::USize K = WallList.GetSize();
	for (GS::USize L = 0; L < K;)
	{ // for each wall
		GS::Array<std::tuple<GS::USize, API_Element, API_Coord>> GuidPoints;
		for (GS::USize i = 0; i < RecHandler::UArc.GetSize(); i++) { // for each arc



			API_ElementMemo memo;
			BNZeroMemory(&memo, sizeof(API_ElementMemo));
			ACAPI_Element_GetMemo(WallList.Get(L), &memo);
			Int32 n = BMhGetSize((GSHandle)memo.coords) / Sizeof32(API_Coord);
			for (Int32 k = 1; k < n; k++) { // for each coord
				if (Line::distanceBetweenPoints((*memo.coords)[k], Line::GetArcBegCoord(RecHandler::UArc.Get(i))) < 0.1 || Line::distanceBetweenPoints((*memo.coords)[k], Line::GetArcEndCoord(RecHandler::UArc.Get(i))) < 0.1)
				{
					GuidPoints.Push(std::make_tuple(i, RecHandler::UArc.Get(i), (*memo.coords)[k]));
					break;
				}
			}


		}

		bool created = false;

		if (GuidPoints.GetSize() == 2) {
			if (Line::distanceBetweenPoints(std::get<2>(GuidPoints.Get(0)), std::get<2>(GuidPoints.Get(1))) < 0.25) {
				if (std::abs(Line::GetAngleFromArc(std::get<1>(GuidPoints.Get(0))) - Line::GetAngleFromArc(std::get<1>(GuidPoints.Get(1)))) < 0.2) {
					//WallArcs.Push(std::make_pair(i, std::make_pair(UArc.Get(i), std::make_pair(GuidPoints.Get(0).first, GuidPoints.Get(1).first))));
					CurWallRec::CreateWallFromArcPair(std::get<1>(GuidPoints.Get(0)), std::get<1>(GuidPoints.Get(1)));
					created = true;
					RecHandler::WallArcs.Push(std::get<1>(GuidPoints.Get(1)));
					RecHandler::WallArcs.Push(std::get<1>(GuidPoints.Get(0)));
					DeleteArcFromUArc(std::get<1>(GuidPoints.Get(1)).header.guid);
					DeleteArcFromUArc(std::get<1>(GuidPoints.Get(0)).header.guid);
				}
			}
		}
		else
		{
			if (GuidPoints.GetSize() > 2) {
				for (GS::USize k = 0; k < GuidPoints.GetSize();) {
					for (GS::USize s = 0; s < GuidPoints.GetSize();) {

						if (Line::distanceBetweenPoints(std::get<2>(GuidPoints.Get(k)), std::get<2>(GuidPoints.Get(s))) < 0.25 && k != s)

						{
							CurWallRec::CreateWallFromArcPair(std::get<1>(GuidPoints.Get(k)), std::get<1>(GuidPoints.Get(s)));
							created = true;
							//WallArcs.Push(std::make_pair(i, std::make_pair(UArc.Get(i), std::make_pair(GuidPoints.Get(k).first, GuidPoints.Get(s).first))));
							RecHandler::WallArcs.Push(std::get<1>(GuidPoints.Get(s)));
							RecHandler::WallArcs.Push(std::get<1>(GuidPoints.Get(k)));
							DeleteArcFromUArc(std::get<1>(GuidPoints.Get(s)).header.guid);
							DeleteArcFromUArc(std::get<1>(GuidPoints.Get(k)).header.guid);
							GuidPoints.Delete(s);
							GuidPoints.Delete(k);


						}
						else s++;

					}
					k++;
				}
			}
		}
		if (created)
		{
			WallList.Clear();
			ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan | APIFilt_OnActFloor);
			K = WallList.GetSize();
			if (L + 2 == K)
				break;
		}
		L++;
	}
	//int NumDeleted = 0;
	//for (GS::USize m = 0; m < WallArcs.GetSize(); m++)
	//{
	//	for (GS::USize n = 0; n < WallArcs.GetSize(); n++)
	//	{
	//		if (m != n)
	//			if (WallArcs.Get(m).second.second.first == WallArcs.Get(n).second.second.first && WallArcs.Get(m).second.second.second == WallArcs.Get(n).second.second.second) {
	//				arcPairs.Push(std::make_pair(WallArcs.Get(m).second.first, WallArcs.Get(n).second.first));
	//				//UArc.Delete(WallArcs.Get(m).first - NumDeleted);
	//				//NumDeleted++;
	//				//UArc.Delete(WallArcs.Get(n).first - NumDeleted);
	//				//NumDeleted++;
	//			}
	//	}

	//}


	/*arcPairs = WallRec::DeleteDupGuidPairs(arcPairs);

	for (GS::USize m = 0; m < arcPairs.GetSize(); m++)*/

}

void CurWallRec::CreateWallFromArcPair(API_Element arc1, API_Element arc2) {
	if (arc1.arc.linePen.penIndex != arc2.arc.linePen.penIndex)
		return;


	API_Coord begC, endC;
	double deg = 0;
	double width = 0;
	width = abs(arc1.arc.r - arc2.arc.r);
	if (width < 0.01)
		width = 0.2;
	if (arc1.arc.r < arc2.arc.r)

	{
		if (!arc1.arc.reflected) {
			begC = (Line::GetArcBegCoord(arc1));
			endC = (Line::GetArcEndCoord(arc1));
		}
		else
		{
			begC = (Line::GetArcEndCoord(arc1));
			endC = (Line::GetArcBegCoord(arc1));
		}
		deg = CurWallRec::GetWallAngleFromArc(arc1);
		CurWallRec::Do_CreateCurvedWall(deg, begC, endC, width);
	}
	else
	{
		if (!arc1.arc.reflected) {
			begC = (Line::GetArcBegCoord(arc2));
			endC = (Line::GetArcEndCoord(arc2));
		}
		else
		{
			begC = (Line::GetArcEndCoord(arc2));
			endC = (Line::GetArcBegCoord(arc2));
		}
		deg = CurWallRec::GetWallAngleFromArc(arc2);
		if (Line::distanceBetweenPoints(begC, endC) < 0.2)
			return;
		CurWallRec::Do_CreateCurvedWall(deg, begC, endC, width);
	}
}


void CurWallRec::DeleteArcFromUArc(API_Guid guid)
{
	for (GS::USize i = 0; i < RecHandler::UArc.GetSize();)
	{
		if (RecHandler::UArc.Get(i).header.guid == guid)
			RecHandler::UArc.Delete(i);
		else i++;
	}
}

double CurWallRec::GetWallAngleFromArc(API_Element arc) {
	double retAng = 0;
	double smallAng = std::min(arc.arc.begAng, arc.arc.endAng);
	double LargeAng = std::max(arc.arc.begAng, arc.arc.endAng);

	if (LargeAng - smallAng <= PI)
		retAng = LargeAng - smallAng;
	else if (PI < (LargeAng - smallAng) && (LargeAng - smallAng) < 2 * PI)
		retAng = LargeAng - smallAng;// -90;
	else retAng = LargeAng;
	if (retAng > (PI * 1.5) - 0.1)
		retAng = 2 * PI - retAng;
	return retAng;
}

//GS::Array<std::pair<API_Element, API_Element>> CurWallRec::DeleteDupGuidPairs(GS::Array<std::pair<API_Element, API_Element>> guidArr) {
//	for (GS::USize i = 0; i < guidArr.GetSize();) {
//		for (GS::USize k = 0; k < guidArr.GetSize();) {
//			if (i != k)
//			{
//				if (guidArr.Get(i).first.header.guid == guidArr.Get(k).first.header.guid) {
//					if (guidArr.Get(i).second.header.guid == guidArr.Get(k).second.header.guid) {
//						guidArr.Delete(k);
//					}
//				}
//				else if (guidArr.Get(i).second.header.guid == guidArr.Get(k).first.header.guid)
//					if (guidArr.Get(i).first.header.guid == guidArr.Get(k).second.header.guid) {
//						guidArr.Delete(k);
//					}
//					else k++;
//			}
//			k++;
//		}
//		i++;
//	}
//	return guidArr;
//
//}

void CurWallRec::CreateCurvedWallsFromIndependentDrawing() {
	GS::Array <std::tuple< GS::USize, API_Element, API_Guid, API_Guid>> WallArcs;
	GS::Array<std::pair<API_Element, API_Element>> arcPairs;
	GS::Array<GS::USize> WallIndArr;
	for (GS::USize k = 0; k < RecHandler::UArc.GetSize();)
	{ // for each wall

		API_Element Curarc = RecHandler::UArc.Get(k);
		API_Coord Curbeg = Line::GetArcBegCoord(Curarc);
		API_Coord Curend = Line::GetArcEndCoord(Curarc);
		double CurAngle = CurWallRec::GetWallAngleFromArc(Curarc);
		GS::Array<std::pair<GS::USize, API_Element>> TestArcs;

		for (GS::USize i = 0; i < RecHandler::UArc.GetSize(); i++) { // for each arc
			API_Element Testarc = RecHandler::UArc.Get(i);
			API_Coord Testbeg = Line::GetArcBegCoord(Testarc);
			API_Coord Testend = Line::GetArcEndCoord(Testarc);
			double TestAngle = CurWallRec::GetWallAngleFromArc(Curarc);
			if (i != k) 
			{
				bool case1 = Line::distanceBetweenPoints(Curbeg, Testbeg) < 0.25 && Line::distanceBetweenPoints(Curend, Testend) < 0.25;
				bool case2 = Line::distanceBetweenPoints(Curbeg, Testend) < 0.25 && Line::distanceBetweenPoints(Curend, Testbeg) < 0.25;
				if (case1 || case2)
				{
					//if (case1)
					//{
					//	/*Line line1 = Line(Curbeg, Testbeg);
					//	Line line2 = Line(Curend, Testend);*/
					//
					//	
					//}
					//else
					//{
					//	/*Line line1 = Line(Curbeg, Testend);
					//	Line line2 = Line(Curend, Testbeg);*/
					//}

					if (abs(TestAngle - CurAngle) < 0.1 && TestAngle != 0)
					{
						TestArcs.Push(std::make_pair(i, Testarc));

					}
				}
			}


		}


		bool created = false;
		if (TestArcs.GetSize() == 1) {

			if (std::abs(CurWallRec::NewGetAngleFromArc(TestArcs.Get(0).second) - CurWallRec::NewGetAngleFromArc(Curarc)) < 0.2) {
				//WallArcs.Push(std::make_pair(i, std::make_pair(UArc.Get(i), std::make_pair(GuidPoints.Get(0).first, GuidPoints.Get(1).first))));
				CurWallRec::CreateWallFromArcPair(TestArcs.Get(0).second, Curarc);
				created = true;
			}

		}

		if (created)
		{
			if (TestArcs.Get(0).first < k)
			{
				RecHandler::UArc.Delete(k);
				RecHandler::UArc.Delete(TestArcs.Get(0).first);
			}
			else
			{
				RecHandler::UArc.Delete(TestArcs.Get(0).first);
				RecHandler::UArc.Delete(k);
			}
		}
		else k++;

		/*else
		{
			if (TestArcs.GetSize() > 1) {
				for (GS::USize t = 0; t < TestArcs.GetSize();) {
					for (GS::USize s = 0; s < TestArcs.GetSize();) {

						if (Line::distanceBetweenPoints(GuidPoints.Get(k).second.second, GuidPoints.Get(s).second.second) < 0.25 && k != s)

						{
							CurWallRec::CreateWallFromArcPair(GuidPoints.Get(k).second.first, GuidPoints.Get(s).second.first);
							created = true;
							//WallArcs.Push(std::make_pair(i, std::make_pair(UArc.Get(i), std::make_pair(GuidPoints.Get(k).first, GuidPoints.Get(s).first))));
							RecHandler::WallArcs.Push(GuidPoints.Get(s).second.first);
							RecHandler::WallArcs.Push(GuidPoints.Get(t).second.first);
							DeleteArcFromUArc(GuidPoints.Get(s).second.first.header.guid);
							DeleteArcFromUArc(GuidPoints.Get(t).second.first.header.guid);
							GuidPoints.Delete(s);
							GuidPoints.Delete(t);


						}
						else s++;

					}
					t++;
				}
			}
		}*/
	}
}

//int NumDeleted = 0;
//for (GS::USize m = 0; m < WallArcs.GetSize(); m++)
//{
//	for (GS::USize n = 0; n < WallArcs.GetSize(); n++)
//	{
//		if (m != n)
//			if (WallArcs.Get(m).second.second.first == WallArcs.Get(n).second.second.first && WallArcs.Get(m).second.second.second == WallArcs.Get(n).second.second.second) {
//				arcPairs.Push(std::make_pair(WallArcs.Get(m).second.first, WallArcs.Get(n).second.first));
//				//UArc.Delete(WallArcs.Get(m).first - NumDeleted);
//				//NumDeleted++;
//				//UArc.Delete(WallArcs.Get(n).first - NumDeleted);
//				//NumDeleted++;
//			}
//	}
//
//}
//
//
//arcPairs = WallRec::DeleteDupGuidPairs(arcPairs);
//
//for (GS::USize m = 0; m < arcPairs.GetSize(); m++)*/

double::CurWallRec::NewGetAngleFromArc(API_Element arc)
{
	API_Coord beg = Line::GetArcBegCoord(arc);
	API_Coord end = Line::GetArcEndCoord(arc);
	beg = arc.arc.origC;
	double	fa = ComputeFiPtr(&arc.arc.origC, &beg);
	double	fe = ComputeFiPtr(&arc.arc.origC, &end);
	while (fa + EPS >= 2 * PI)
		fa -= 2 * PI;
	while (fe + EPS >= 2 * PI)
		fe -= 2 * PI;
	 return fa - fe;
}