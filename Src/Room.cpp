#include "Room.h"
#include "RoomRec.h"
#include "Numberer.h"

//private member initilizaton - don't touch

//private member initilizaton - don't touch
GS::Array<Room> RoomList::List;
bool RoomList::IsInstantiated = false;

Room::Room()
{




	GS::Array<API_Coord> shape;
	API_Guid fill = APINULLGuid;
	Room::SetShape(shape, fill);
	(*this).FloorInd = Room::GetFloorInd(Room::ShapeGuid);
}

void Room::SetRoomElements() {
	GS::Array<API_Guid> WallList;
	GSErrCode err = ACAPI_Element_GetElemList(API_WallID, &WallList, APIFilt_OnVisLayer | APIFilt_FromFloorplan );
	for (GS::Array<API_Guid>::ConstIterator it = WallList.Enumerate(); it != nullptr; ++it)
	{ // for each wall
		API_Element element;
		BNZeroMemory(&element, sizeof(API_Element));
		element.header.guid = *it;
		err = ACAPI_Element_Get(&element);

		for (GS::USize i = 0; i < (*this).Shape.GetSize(); i++)
		{
			if (Line::distanceBetweenPoints(element.wall.begC, (*this).Shape.Get(i)) < 0.1 || Line::distanceBetweenPoints(element.wall.endC, (*this).Shape.Get(i)) < 0.1)
			{
				(*this).RoomElements.Push(element.header.guid);
				break;
			}
		
		}


	}


}

GS::Array<API_Guid> Room::GetRoomElements() {
	return (*this).RoomElements;
}


Room::Room(GS::Array<API_Coord> shape, bool CreateText, bool CreateFill, bool AutoNaming)
{
	if (CreateFill)
		Room::SetShape(shape, FloorRec::CreateFillFromCoordArr(shape, RoomRec::CreateRandShort(1, 120)));
	else
		Room::SetShape(shape, APINULLGuid);
	(*this).FloorInd = Room::GetFloorInd(Room::ShapeGuid);
	Room::SetRoomElements();
	if (CreateText)
		Room::CreateText(AutoNaming);
}

Room::Room(GS::Array<API_Coord> shape, API_Guid fill, bool CreateText, bool CreateFill, bool AutoNaming)
{
	if (CreateFill)
		Room::SetShape(shape, fill);
	else {
		Room::SetShape(shape, APINULLGuid);
		GS::Array<API_Guid> del;
		del.Push(fill);
		ACAPI_CallUndoableCommand("Change Mesh",
			[&]() -> GSErrCode {
				return ACAPI_Element_Delete(del);
			});
	}
	(*this).FloorInd = Room::GetFloorInd(Room::ShapeGuid);
	Room::SetRoomElements();
	if (CreateText)
		Room::CreateText(AutoNaming);
}

std::pair<GS::Array<API_Coord>, API_Guid> Room::GetShape()
{
	return std::make_pair((*this).Shape, (*this).ShapeGuid);
}

void Room::SetShape(GS::Array<API_Coord> shape, API_Guid guid)
{
	(*this).Shape = shape;
	(*this).ShapeGuid = guid;
	(*this).area = FloorRec::GetElemArea((*this).ShapeGuid);
}

void Room::SetShape(GS::Array<API_Coord> shape)
{
	(*this).Shape = shape;
	(*this).ShapeGuid = FloorRec::CreateFillFromCoordArr(Room::Shape, RoomRec::CreateRandShort(1, 120));
	(*this).area = FloorRec::GetElemArea(Room::ShapeGuid);
}

API_Guid Room::GetText()
{
	return (*this).TextGuid;
}

void Room::SetText(API_Guid textGuid)
{

	(*this).TextGuid = textGuid;
}

void Room::SetFloorInd(short FloorInd)
{
	(*this).FloorInd = FloorInd;
}

short Room::GetFloorInd()
{
	return (*this).FloorInd;
}

short Room::GetFloorInd(API_Guid guid) {
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = guid;
	ACAPI_Element_Get(&element);
	return element.header.floorInd;

}

double Room::GetRoomArea()
{
	return (*this).area;
}

void Room::CreateText(bool AutoNaming)
{
	API_Guid Textg = APINULLGuid;
	if (AutoNaming)
		Textg = Room::Do_Create3DText(RoomRec::GetArrMidPoint(Room::Shape), Room::AutoNaming(Room::area), Room::GetFloorInd(), true);
	else  Textg = Room::Do_Create3DText(RoomRec::GetArrMidPoint(Room::Shape), GS::UniString(L"חדר", CC_Hebrew), Room::GetFloorInd(), true);

	Room::SetText(Textg);
}

API_Guid	Room::Do_Create3DText(API_Coord point, GS::UniString text, short floorInd, bool InsideWall)
{
	GSErrCode err;
	//API_ParamOwnerType   paramOwner;
	//API_ChangeParamType  chgParam;
	//API_GetParamsType    getParams;
	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	API_LibPart libPart;
	BNZeroMemory(&libPart, sizeof(libPart));
	double               a, b;
	Int32                addParNum;
	API_AddParType** addPars = nullptr;

	element.header.typeID = API_ObjectID;
	element.header.variationID = APIVarId_Object;
	err = ACAPI_Element_GetDefaults(&element, &memo);
	if (InsideWall)
		GS::ucscpy(libPart.docu_UName, L("Text Opening M"));
	else
		GS::ucscpy(libPart.docu_UName, L("Text Opening M"));
	if (ACAPI_LibPart_Search(&libPart, false) != NoError) // find in the libraries,
	{
		ACAPI_WriteReport("Error in Finding library part: Text Opening ", false);
		return APINULLGuid;
	}
	ACAPI_LibPart_Get(&libPart);
	err = ACAPI_LibPart_GetParams(libPart.index, &a, &b, &addParNum, &addPars);
	memo.params = addPars;
	element.object.libInd = libPart.index;
	element.object.pos = point;
	element.header.floorInd = floorInd;
	element.object.mat = 242;
	err = ACAPI_CallUndoableCommand("Create Text",
		[&]() -> GSErrCode {
			return ACAPI_Element_Create(&element, &memo);
		});
	if (err != NoError) {
		ACAPI_WriteReport("ACAPI_Element_Create (3d text)", false, err);
	}
	Numberer::ChangeParamValue(element, "rotAngleX", 0);
	Numberer::ChangeParamValue(element, "F", text);
	return element.header.guid;



}



RoomList::RoomList()
{
	IsInstantiated = true;
	GS::Array<Room> list;
	RoomList::List = list;
}

GS::Array<Room> RoomList::GetList()
{
	if (!IsInstantiated)
		RoomList::RoomList();
	return RoomList::List;
}

void RoomList::AddToList(Room room)
{
	GS::Array<Room> list = RoomList::GetList();
	list.Push(room);
	RoomList::List = list;
}

void RoomList::RemoveFromList(GS::UIndex index)
{
	GS::Array<Room> list = RoomList::GetList();
	Room curRoom = list.Get(index);
	if (curRoom.TextGuid != APINULLGuid)
	{
		GS::Array<API_Guid> del;
		del.Push(curRoom.TextGuid);
		GSErrCode err = ACAPI_CallUndoableCommand("Change Mesh",
			[&]() -> GSErrCode {
				return ACAPI_Element_Delete(del);
			});
		if (err != NoError) {
			ACAPI_WriteReport("Failed Deleting Text", false);
		}
	}

	if (curRoom.ShapeGuid != APINULLGuid)
	{
		GS::Array<API_Guid> del;
		del.Push(curRoom.ShapeGuid);
		GSErrCode err = ACAPI_CallUndoableCommand("Change Mesh",
			[&]() -> GSErrCode {
				return ACAPI_Element_Delete(del);
			});
		if (err != NoError) {
			ACAPI_WriteReport("Failed Deleting Fill", false);
		}
	}
	list.Delete(index);
	RoomList::List = list;
}

void RoomList::DeleteAll() {
	while (true) {
		if (RoomList::List.GetSize() == 0)
			break;
		RoomList::RemoveFromList(RoomList::List.GetSize() - 1);
	}
}

GS::UniString Room::AutoNaming(double area) {



	if (area == 0)
		return GS::UniString(L"חדר", CC_Hebrew);

	for (GS::USize i = 0; i < (*this).RoomElements.GetSize(); i++)
	{
		
			API_Element element;
			BNZeroMemory(&element, sizeof(API_Element));
			element.header.guid = (*this).RoomElements.Get(i);
			ACAPI_Element_Get(&element);
			if(element.wall.height < 2.1)
				return GS::UniString(L"מרפסת", CC_Hebrew);

	} //ifBalcony

	if (area > 0 && area < 5)
		return GS::UniString(L"שירותים", CC_Hebrew);
	if (area > 5 && area < 20)
		return GS::UniString(L"חדר", CC_Hebrew);
	if (area > 20 && area < 40)
		return GS::UniString(L"חלל ציבורי", CC_Hebrew);
	if (area > 40)
		return GS::UniString(L"חלל אחסון", CC_Hebrew);
	return GS::UniString(L"חדר", CC_Hebrew);
}

