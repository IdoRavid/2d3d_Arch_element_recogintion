#include "RoomRec.h"
#include "FloorRec.h"
#include <random>
#include  "Polygons.h"
//#include "TmrTech.h"
#include "Numberer.h"
#include "Room.h"



void RoomRec::RecognizeRooms(bool WithText, bool WithFill, bool AutoNaming) {
	GS::UniString Title = GS::UniString("Creating Rooms");
	GS::UniString SubT1 = GS::UniString("Throwing Dots");
	GS::UniString SubT2 = GS::UniString("Creating Fills");

	short n = 2;
	short n1 = 1;
	short n2 = 2;
	ACAPI_Interface(APIIo_InitProcessWindowID, &Title, &n);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &SubT1, &n1);
	short FloorInd = RoomRec::GetCurrentFloor();
	GS::Array<API_Coord> pArr, floor;
	floor = FloorRec::CreateFloorcArr();




	API_Guid FillGuid = FloorRec::CreateFillFromCoordArr(floor, 19);
	//create fill
	double area = FloorRec::GetElemArea(FillGuid);
	if (area == 0 || FillGuid == APINULLGuid) {
		API_Coord a, b, c, d;
		a.x = 100, a.y = 100;
		b.x = -100, b.y = 100;
		c.x = -100, c.y = -100;
		d.x = 100, d.y = -100;
		floor.Push(a);
		floor.Push(b);
		floor.Push(c);
		floor.Push(d);
		/*GS::UniString can = GS::UniString("Canceled! Couldn't Find Contour");
		ACAPI_Interface(APIIo_SetProcessValueID, &can, nullptr);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
		return;*/
	}
	GS::Array<API_Guid> del;
	del.Push(FillGuid);
	ACAPI_CallUndoableCommand("Change Mesh",
		[&]() -> GSErrCode {
			return ACAPI_Element_Delete(del);
		});


	GS::USize size = 100;
	if (FillGuid == APINULLGuid)
		size = 200;
	while (pArr.GetSize() < size) {
		double val = pArr.GetSize();
		ACAPI_Interface(APIIo_SetProcessValueID, &val, nullptr);
		API_Coord point = RoomRec::GenRandCoordAtRadius(RoomRec::GetArrMidPoint(floor), RoomRec::DistanceOfFurthestPoint(floor));
		//Extras::Do_CreateCircle(point);
		if (RoomRec::IsPointInPoly(point, floor))
		{// only if the Coord isnt inside a wall, and is inside the floor polygon, add it
			pArr.Push(point);
		}

	}
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &SubT2, &n2);
	
	
	for (GS::USize i = 0; i < pArr.GetSize(); i++)
	{
		GS::Array<Room> rList = RoomList::GetList();
		double val = i;
		ACAPI_Interface(APIIo_SetProcessValueID, &val, nullptr);
		bool goodP = true;
		
		API_Coord p = pArr.Get(i);
		if (rList.GetSize() > 0) {
			for (GS::USize j = 0; j < (rList.GetSize()); j++) {
				GS::Array<API_Coord> shape = rList.Get(j).GetShape().first;
				short roomFloor = (rList).Get(j).GetFloorInd();
				if (RoomRec::IsPointInPoly(p, shape) && FloorInd == roomFloor) {
					goodP = false;
				}
			}
		}
		//if point isnt in any room:
		if (goodP) {
			GS::Array<API_Coord> roomShape = RoomRec::CreateRoom(p); //try to create room
			if (roomShape.GetSize() > 0) // if succeded
			{
				API_Guid g = FloorRec::CreateFillFromCoordArr(roomShape, RoomRec::CreateRandShort(1, 120));
				if (g != APINULLGuid) {
					RoomList::AddToList((Room(roomShape, g, WithText, WithFill, AutoNaming))); //add to room Array
				}
			}
		}
	}

	ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
}		


API_Guid RoomRec::GetClosestPoint(GS::Array<std::pair<API_Guid, API_Coord>> pair1, API_Coord point) {
	double dist = 100000;
	API_Guid OurGuid = APINULLGuid;
	for (int it = 0; it < int(pair1.GetSize()); it++)
	{
		API_Coord begC = pair1.Get(it).second;
		double begdist = sqrt(((point.x - begC.x) * (point.x - begC.x)) + ((point.y - begC.y) * (point.y - begC.y)));
		if (begdist < dist) {
			OurGuid = pair1.Get(it).first;
			dist = begdist;
		}

	}
	return OurGuid;
}

API_Coord RoomRec::RayCast(Line ray, Line hit) {

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

GS::Array<API_Guid> RoomRec::DeleteDupGuids(GS::Array<API_Guid> cArr) {
	if (cArr.GetSize() < 2)
		return cArr;
	//if unique 
	if (cArr.GetFirst() != cArr.GetLast() && cArr.GetFirst() != cArr.Get(1)) // if first unique
		if (cArr.GetFirst() != cArr.GetLast() && cArr.GetFirst() != cArr.Get(cArr.GetSize() - 2))
			if (cArr.GetFirst() != cArr.Get(1) && cArr.GetFirst() != cArr.Get(2))
				cArr.Delete(0);

	if (cArr.Get(1) != cArr.GetFirst() && cArr.Get(1) != cArr.Get(2)) // if second unique
		if (cArr.Get(1) != cArr.GetLast() && cArr.Get(1) != cArr.Get(0))
			if (cArr.Get(1) != cArr.Get(2) && cArr.Get(1) != cArr.Get(3))
				cArr.Delete(1);

	if (cArr.GetLast() != cArr.Get(cArr.GetSize() - 2) && cArr.GetLast() != cArr.GetFirst()) // if last unique
		if (cArr.GetLast() != cArr.Get(cArr.GetSize() - 2) && cArr.GetLast() != cArr.Get(cArr.GetSize() - 3))
			if (cArr.GetLast() != cArr.Get(1) && cArr.GetLast() != cArr.Get(0))
				cArr.Delete(cArr.GetSize() - 1);

	if (cArr.Get(cArr.GetSize() - 2) != cArr.Get(cArr.GetSize() - 3) && cArr.GetLast() != cArr.Get(cArr.GetSize() - 1)) // if last unique
		if (cArr.Get(cArr.GetSize() - 2) != cArr.Get(cArr.GetSize() - 1) && cArr.Get(cArr.GetSize() - 2) != cArr.Get(0))
			if (cArr.Get(cArr.GetSize() - 2) != cArr.Get(cArr.GetSize() - 3) && cArr.Get(cArr.GetSize() - 2) != cArr.Get(cArr.GetSize() - 4))
				cArr.Delete(cArr.GetSize() - 2);

	for (int i = 2; i < int(cArr.GetSize()) - 2;)
	{
		if (cArr.Get(i) != cArr.Get(i - 1) && cArr.Get(i) != cArr.Get(i + 1)) {// if second unique
			if (cArr.Get(i) != cArr.Get(i - 1) && cArr.Get(i) != cArr.Get(i - 2)) {
				if (cArr.Get(i) != cArr.Get(i + 1) && cArr.Get(i) != cArr.Get(i + 2)) {
					cArr.Delete(i);
				}
				else i++;
			}
			else i++;
		}
		else i++;
	}


	//if duplicates
	for (int i = 0; i < int(cArr.GetSize()) - 1;)
	{
		API_Guid Cur = cArr.Get(i);
		API_Guid nextC = cArr.Get(i + 1);
		if (Cur == nextC)
		{
			cArr.Delete(i + 1);
		}
		else
			i++;
	}
	API_Guid Cur = cArr.GetFirst();
	API_Guid nextC = cArr.GetLast();

	if (Cur == nextC)
		cArr.Delete(cArr.GetSize() - 1);
	return cArr;
}

GS::Array<API_Guid> RoomRec::NewRaycastToWalls(API_Coord point, double raynum) {
	//def lines
	GS::Array<API_Guid>  SelectedWalls;

	for (int i = 0; i < raynum; i++)
	{
		GS::Array<std::pair<API_Guid, API_Coord>> hit;
		API_Element element;
		double deg = i * (360 / raynum);
		Line line = Line::CreateTangentByDeg(point, deg); // line by defined deg


		GS::Array<API_Guid> WallList;
		GSErrCode err = ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan);
		for (GS::Array<API_Guid>::ConstIterator it = WallList.Enumerate(); it != nullptr; ++it)
		{ // for each wall
			BNZeroMemory(&element, sizeof(API_Element));
			element.header.guid = *it;
			err = ACAPI_Element_Get(&element);

			API_Coord test = RoomRec::RayCast(line, Line(element.wall.begC, element.wall.endC)); //creating a hit
			hit.Push(std::make_pair(element.header.guid, test));
		}
		//select test
		if ((0 <= deg && deg < 90) || (270 < deg && deg <= 360)) {
			for (GS::USize i = 0; i < hit.GetSize();)
			{
				API_Coord p = hit.Get(i).second;
				if (p.x < point.x) {
					hit.Delete(i);
				}
				else i++;
			}
		}

		if (90 < deg && deg < 270) {
			for (GS::USize i = 0; i < hit.GetSize();)
			{
				API_Coord p = hit.Get(i).second;
				if (p.x > point.x) {
					hit.Delete(i);
				}
				else i++;
			}
		}
		if (deg == 90) {
			for (GS::USize i = 0; i < hit.GetSize();)
			{
				API_Coord p = hit.Get(i).second;
				if (p.y < point.y) {
					hit.Delete(i);
				}
				else i++;
			}
		}

		if (deg == 270) {
			for (GS::USize i = 0; i < hit.GetSize();)
			{
				API_Coord p = hit.Get(i).second;
				if (p.y > point.y) {
					hit.Delete(i);
				}
				else i++;
			}
		}


		API_Guid guid2 = RoomRec::GetClosestPoint(hit, point);
		if (guid2 != APINULLGuid)
			SelectedWalls.Push(guid2);


	}
	SelectedWalls = RoomRec::DeleteDupGuids(SelectedWalls);
	return SelectedWalls;
}

GS::Array<API_Coord> RoomRec::CreateIntresectArr(GS::Array<API_Guid> gArr) {
	GS::Array<API_Coord> cArr, emptyArr;
	double mindist = 0.2;
	if (gArr.GetSize() < 2)
		return emptyArr;
	for (int i = 0; i < int(gArr.GetSize()) - 1; i++)
	{
		API_Element Cur, NextE;
		BNZeroMemory(&Cur, sizeof(API_Element));
		BNZeroMemory(&NextE, sizeof(API_Element));
		Cur.header.guid = gArr.Get(i);
		NextE.header.guid = gArr.Get(i + 1);
		ACAPI_Element_Get(&Cur);
		ACAPI_Element_Get(&NextE);
		Line CLine(Cur.wall.begC, Cur.wall.endC);
		Line NLine(NextE.wall.begC, NextE.wall.endC);
		API_Coord MeetP = Line::LineMeetingPoint(CLine, NLine);
		if (Line::IsPointOnRealPartOfLine(MeetP, CLine.beg, CLine.end) && Line::IsPointOnRealPartOfLine(MeetP, NLine.beg, NLine.end))
			cArr.Push(MeetP);
		else
			if (Line::distanceBetweenPoints(MeetP, CLine.beg) < mindist || Line::distanceBetweenPoints(MeetP, CLine.end) < mindist || Line::distanceBetweenPoints(MeetP, NLine.beg) < mindist || Line::distanceBetweenPoints(MeetP, NLine.end) < mindist)
				cArr.Push(MeetP);
			else return emptyArr;

	}
	//first-last pair
	API_Element Cur, NextE;
	BNZeroMemory(&Cur, sizeof(API_Element));
	BNZeroMemory(&NextE, sizeof(API_Element));
	Cur.header.guid = gArr.GetFirst();
	NextE.header.guid = gArr.GetLast();
	ACAPI_Element_Get(&Cur);
	ACAPI_Element_Get(&NextE);
	Line CLine(Cur.wall.begC, Cur.wall.endC);
	Line NLine(NextE.wall.begC, NextE.wall.endC);
	API_Coord MeetP = Line::LineMeetingPoint(CLine, NLine);
	if (Line::IsPointOnRealPartOfLine(MeetP, CLine.beg, CLine.end) && Line::IsPointOnRealPartOfLine(MeetP, NLine.beg, NLine.end))
		cArr.Push(MeetP);
	else
		if (Line::distanceBetweenPoints(MeetP, CLine.beg) < mindist || Line::distanceBetweenPoints(MeetP, CLine.end) < mindist || Line::distanceBetweenPoints(MeetP, NLine.beg) < mindist || Line::distanceBetweenPoints(MeetP, NLine.end) < mindist)
			cArr.Push(MeetP);
	return cArr;


}

void RoomRec::WallRayCastTest(bool WithText, bool WithFill, bool AutoNaming) {
	API_Coord c;
	GS::Array <API_Guid> Sel, tryArr;
	GS::Array <API_Coord> cArr;
	GS::Array <API_Neig> SelN;
	if (!ClickAPoint("Click on the vertexes of the Line", &c))
		return;
	Sel = RoomRec::NewRaycastToWalls(c, 360);
	/*for (GS::USize i = 0; i < Sel.GetSize(); i++)
	{
		SelN.Push(API_Neig(Sel.Get(i)));
	}
	ACAPI_Element_Select(SelN, true);*/
	cArr = RoomRec::CreateIntresectArr(Sel);
	if (cArr.GetSize() == 0)
	{
		tryArr = RoomRec::DeleteRepetetiveGuidsHigh(Sel);
		cArr = RoomRec::CreateIntresectArr(tryArr);
		if (cArr.GetSize() == 0) {
			tryArr = RoomRec::DeleteRepetetiveGuidsLow(Sel);
			cArr = RoomRec::CreateIntresectArr(tryArr);
			if (cArr.GetSize() == 0) {
				ACAPI_WriteReport("Invalid Poly", true);
				return;
			}
		}
	}
	RoomList::AddToList(Room(cArr, FloorRec::CreateFillFromCoordArr(cArr, 6), WithText, WithFill, AutoNaming));

}


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

API_Coord RoomRec::GenRandCoord(double lower_bound, double upper_bound) {
	API_Coord a;
	a.x = random<double>(lower_bound, upper_bound);
	a.y = random<double>(lower_bound, upper_bound);
	return a;
}

short RoomRec::CreateRandShort(short low, short high)
{
	return random<short>(low, high);
}



GS::Array <API_Coord>  RoomRec::CreateRoom(API_Coord c) {

	GS::Array <API_Guid> Sel, tryArr;
	GS::Array <API_Coord> cArr, emptyArr;
	GS::Array <API_Neig> SelN;
	Sel = RoomRec::NewRaycastToWalls(c, 360);
	for (GS::USize i = 0; i < Sel.GetSize(); i++)
	{
		SelN.Push(API_Neig(Sel.Get(i)));
	}
	//ACAPI_Element_Select(SelN, true);
	cArr = RoomRec::CreateIntresectArr(Sel);
	if (cArr.GetSize() == 0)
	{

				ACAPI_WriteReport("Invalid Poly", false);
				return emptyArr;
	}
	return cArr;




}

short RoomRec::GetCurrentFloor() {
	GSErrCode           err;
	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
	return storyInfo.actStory;
}

API_Coord RoomRec::GenRandCoordAtRadius(API_Coord point, double Radius) {
	double dist = INFINITY;
	API_Coord p;
	p.x = INFINITY, p.y = INFINITY;
	while (Radius < dist) {
		p = RoomRec::GenRandCoord(-35, +35);
		dist = Line::distanceBetweenPoints(point, p);
	}
	return p;
}

API_Coord RoomRec::GetArrMidPoint(GS::Array<API_Coord> cArr) {
	API_Coord Avg;
	Avg.x = 0, Avg.y = 0;
	if (cArr.GetSize() == 0)
		return Avg;
	if (cArr.GetSize() == 1)
		return cArr.GetFirst();
	for (GS::USize i = 0; i < cArr.GetSize(); i++)
	{
		Avg.x += cArr.Get(i).x;
		Avg.y += cArr.Get(i).y;
	}
	Avg.x = Avg.x / cArr.GetSize();
	Avg.y = Avg.y / cArr.GetSize();
	return Avg;
}

double RoomRec::DistanceOfFurthestPoint(GS::Array<API_Coord> cArr) {
	API_Coord Mid = RoomRec::GetArrMidPoint(cArr);
	double dist = 0;
	if (cArr.GetSize() == 0)
		return dist;
	if (cArr.GetSize() == 1)
		return dist;
	for (GS::USize i = 0; i < cArr.GetSize(); i++)
	{
		double OurDist = Line::distanceBetweenPoints(cArr.Get(i), Mid);
		if (dist < OurDist) {
			dist = OurDist;
		}
	}

	return dist;
}

bool RoomRec::IsPointInPoly(API_Coord point, GS::Array<API_Coord> poly) {
	GS::Array<API_Coord> hit;
	double deg = 0;
	Line line = Line::CreateTangentByDeg(point, deg); // line by defined deg


	for (GS::USize i = 0; i < poly.GetSize() - 1; i++)
	{ // for each wall

		API_Coord test = RoomRec::RayCast(line, Line(poly.Get(i), poly.Get(i + 1))); //creating a hit
		if (test.x != INFINITY && test.y != INFINITY) //checking validity
			hit.Push(test);
	}
	API_Coord test = RoomRec::RayCast(line, Line(poly.GetFirst(), poly.GetLast())); //creating a hit
	if (test.x != INFINITY && test.y != INFINITY) //checking validity
		hit.Push(test);

	for (GS::USize i = 0; i < hit.GetSize();)
	{
		API_Coord p = hit.Get(i);
		if (p.x < point.x) {
			hit.Delete(i);
		}
		else i++;
	}
	if (hit.GetSize() % 2 == 0)
		return false;
	else return true;

}

GS::Array<API_Guid>  RoomRec::DeleteRepetetiveGuidsHigh(GS::Array<API_Guid> gArr) {
	for (GS::USize i = 0; i < gArr.GetSize(); i++)
	{
		for (GS::USize j = 0; j < gArr.GetSize();)
		{
			if (gArr.Get(i) == gArr.Get(j) && i < j)
				gArr.Delete(j);
			else j++;
		}
	}
	return gArr;
}
GS::Array<API_Guid>  RoomRec::DeleteRepetetiveGuidsLow(GS::Array<API_Guid> gArr) {
	for (GS::USize i = gArr.GetSize() -1; i > 0; i--)
	{
		API_Guid Ig = gArr.Get(i);
		for (GS::USize j = gArr.GetSize() -1; j > 0;)
		{
			API_Guid Jg =  gArr.Get(j);;
			if (Jg == Ig && i > j) {
				gArr.Delete(j);
				j--;
				if (i == gArr.GetSize() + 1)
					i--;
			}
			else j--;
		}
	}
	return gArr;
}

void RoomRec::RecognizeAllFloors(bool WithText, bool WithFill, bool AutoNaming) {

	GSErrCode           err;
	API_StoryInfo storyInfo;
	BNZeroMemory(&storyInfo, sizeof(API_StoryInfo));
	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);

	for (short i = storyInfo.firstStory; i < storyInfo.lastStory +1; i++)
	{
		int wallsInStory = 0;
		GS::Array<API_Guid> WallList;
		GSErrCode err = ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan);
		for (GS::Array<API_Guid>::ConstIterator it = WallList.Enumerate(); it != nullptr; ++it)
		{ // for each wall
			API_Element element;
			BNZeroMemory(&element, sizeof(API_Element));
			element.header.guid = *it;
			err = ACAPI_Element_Get(&element);
			if (element.header.floorInd == i)
				wallsInStory++;
		}
		if (wallsInStory > 3) {
			API_StoryCmdType storyCmd;
			BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
			storyCmd.index = i;
			storyCmd.action = APIStory_GoTo;
			err = ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd);
			RoomRec::RecognizeRooms(WithText, WithFill, AutoNaming);
		}
	}

}

void RoomRec::DeleteRoom() {

	API_Coord c;
	if (!ClickAPoint("Click on the vertexes of the Line", &c))
		return;

	for (GS::USize j = 0; j < (RoomList::GetList()).GetSize(); j++) {
		if (RoomRec::IsPointInPoly(c, (RoomList::GetList()).Get(j).GetShape().first) && RoomRec::GetCurrentFloor() == (RoomList::GetList()).Get(j).GetFloorInd()) {
		
			RoomList::RemoveFromList(j);
		}

	}
}

//ROOM Wizard GUI


const short dialID = 32426;
const short closeButton = 1, TextsCB = 2, FillsCB = 3, AutoNameCB = 4, AllFloorsRadio = 5, SingleFloorRadio = 6, ManualRoomCreate = 7, ManualRoomDelete = 8, AutoRoomCreate = 9, DeleteAllRooms = 10;

void RoomRec::Do_RoomWizard(void) {
	DGModelessInit(ACAPI_GetOwnResModule(), dialID, ACAPI_GetOwnResModule(), RoomRec::APIRoomWizard_Handler, 0, 1);//Opening the obj export form.
	DGEnableMessage(dialID, DG_ALL_ITEMS, DG_MSG_DRAGDROP);
}

GSErrCode __ACENV_CALL	RoomRec::APIRoomWizardPaletteAPIControlCallBack(Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
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

short DGCALLBACK RoomRec::APIRoomWizard_Handler(short message, short dialogID, short item, DGUserData /*userData*/, DGMessageData /*msgData*/)
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
		DGSetItemValLong(dialogID, TextsCB, 1);
		DGSetItemValLong(dialogID, FillsCB, 1);
		DGSetItemValLong(dialogID, AutoNameCB, 1);
		DGSetItemValLong(dialogID, AllFloorsRadio, 1);
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

	
		case ManualRoomCreate:
		{
			RoomRec::WallRayCastTest(DGGetItemValLong(dialogID, TextsCB), DGGetItemValLong(dialogID, FillsCB), DGGetItemValLong(dialogID, AutoNameCB));
			break;
		}
		case AutoRoomCreate:
		{
			DGSetItemValLong(dialogID, AutoRoomCreate, 0);
			if (DGGetItemValLong(dialogID, AllFloorsRadio))
				RoomRec::RecognizeAllFloors(DGGetItemValLong(dialogID, TextsCB), DGGetItemValLong(dialogID, FillsCB), DGGetItemValLong(dialogID, AutoNameCB));
			else
				RoomRec::RecognizeRooms(DGGetItemValLong(dialogID, TextsCB), DGGetItemValLong(dialogID, FillsCB), DGGetItemValLong(dialogID, AutoNameCB));
			break;
		}
		case DeleteAllRooms:
		{
			RoomList::DeleteAll();
			break;
		}
		case ManualRoomDelete:
		{
			RoomRec::DeleteRoom();
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
	}// APIRoomWizard_Handler
	return ret;
}
