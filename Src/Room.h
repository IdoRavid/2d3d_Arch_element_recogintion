#pragma once
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
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
#include <map>
#include "FloorRec.h"

class Room
{
	public:
	
		 GS::Array<API_Coord> Room::Shape;
		 API_Guid Room::ShapeGuid;
		 API_Guid Room::TextGuid;
		 GS::Array<API_Guid> Room::RoomElements;
		 short Room::FloorInd;
		 double Room::area;

		void Room::SetFloorInd(short FloorInd);
		void Room::SetText(API_Guid textGuid);
		 void Room::SetShape(GS::Array<API_Coord> shape, API_Guid guid);
		 void Room::SetShape(GS::Array<API_Coord> shape);
		 void CreateText(bool AutoNaming);
		static API_Guid Do_Create3DText(API_Coord point, GS::UniString text, short floorInd, bool InsideWall);

		GS::UniString AutoNaming(double area);

	Room::Room();
	void SetRoomElements();
	GS::Array<API_Guid> GetRoomElements();
	Room::Room(GS::Array<API_Coord> shape, bool CreateText, bool CreateFill, bool AutoNaming);
	Room::Room(GS::Array<API_Coord> shape, API_Guid fill, bool CreateText, bool CreateFill, bool AutoNaming);
	 std::pair< GS::Array<API_Coord>, API_Guid>  Room::GetShape();
	API_Guid Room::GetText();
	short Room::GetFloorInd();
	static short GetFloorInd(API_Guid guid);
	double GetRoomArea();

};

class RoomList {
private:
	static GS::Array<Room> List;
	static bool IsInstantiated;
public:
	 RoomList::RoomList();
	static GS::Array<Room> RoomList::GetList();
	static void RoomList::AddToList(Room room);
	static void RoomList::RemoveFromList(GS::UIndex index);

	static void DeleteAll();

};
