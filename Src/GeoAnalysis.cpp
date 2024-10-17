#include "GeoAnalysis.h"
#include "APIEnvir.h"
#include "WallRec.h"


// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include "Extras.h"
#include	"APICommon.h"
#include <cmath>
#include "vector"
#include <algorithm>

Line::Line(API_Guid guid) {



	API_Element element;
	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = guid;
	ACAPI_Element_Get(&element);
	Line::guid = guid;
	Line::layer = element.header.layer;
	Line::ParentLines.first = APINULLGuid;
	Line::ParentLines.second = APINULLGuid;
	API_Coord a, b;
	if (element.header.typeID == API_WallID)
	{
		PenInd = 0;
		 a = element.wall.begC;
		 b = element.wall.endC;
	}
	else 
	{
		PenInd = WallRec::ReturnLinePenInd(guid);
		 a = element.line.begC;
		 b = element.line.endC;
	}
	
	
	beg = a;
	end = b;
	if (abs(b.x - a.x) > 0.001 && abs(b.y - a.y) > 0.001) {
		m = ((b.y) - (a.y)) / ((b.x) - (a.x));
		c = b.y - m * b.x;
	}
	else
	{
		if (abs(b.x - a.x) < 0.001)
		{
			ParallelToY = true;
			m = 0;
			c = 0;
		}
		else
		{
			ParallelToX = true;
			m = 0;
			c = 0;
		}


	}

}
Line::Line(API_Coord a, API_Coord b, Line ParentA, Line ParentB)
{
	if (Line::distanceBetweenPoints(a, b) < 0.005) {
		Line();
		return;
	}
	Line::guid = {};
	Line::ParentLines.first = ParentA.guid;
	Line::ParentLines.second = ParentB.guid;
	Line::PenInd = 0;
	Line::layer = 0;
	beg = a;
	end = b;
	if (abs(b.x - a.x) > 0.001 && abs(b.y - a.y) > 0.001) {
		m = ((b.y) - (a.y)) / ((b.x) - (a.x));
		c = b.y - m * b.x;
	}
	else
	{
		if (abs(b.x - a.x) < 0.001)
		{
			ParallelToY = true;
			m = 0;
			c = 0;
		}
		else
		{
			ParallelToX = true;
			m = 0;
			c = 0;
		}
	}
}

Line::Line(API_Coord a, API_Coord b)
{
	if (Line::distanceBetweenPoints(a, b) < 0.005) {
		Line();
		return;
	}
	Line::guid = {};
	Line::ParentLines.first = APINULLGuid;
	Line::ParentLines.second = APINULLGuid;
	Line::PenInd = 0;
	Line::layer = 0;
	beg = a;
	end = b;
	if (abs(b.x - a.x) > 0.001 && abs(b.y - a.y) > 0.001) {
		m = ((b.y) - (a.y)) / ((b.x) - (a.x));
		c = b.y - m * b.x;
	}
	else
	{
		if (abs(b.x - a.x) < 0.001)
		{
			ParallelToY = true;
			m = 0;
			c = 0;
		}
		else
		{
			ParallelToX = true;
			m = 0;
			c = 0;
		}
	}
}

Line::Line(API_Coord a, API_Coord b, API_Element poly)
{
	if (Line::distanceBetweenPoints(a, b) < 0.005) {
		Line();
		return;
	}
	Line::guid = poly.header.guid;
	Line::PenInd = poly.polyLine.linePen.penIndex;
	Line::layer = poly.header.layer;
	Line::ParentLines.first = APINULLGuid;
	Line::ParentLines.second = APINULLGuid;
	beg = a;
	end = b;
	if (abs(b.x - a.x) > 0.001 && abs(b.y - a.y) > 0.001) {
		m = ((b.y) - (a.y)) / ((b.x) - (a.x));
		c = b.y - m * b.x;
	}
	else
	{
		if (abs(b.x - a.x) < 0.001)
		{
			ParallelToY = true;
			m = 0;
			c = 0;
		}
		else
		{
			ParallelToX = true;
			m = 0;
			c = 0;
		}
	}
}

Line::Line()
{
	Line::guid = {};
	Line::PenInd = -1;
	Line::ParentLines.first = {};
	Line::ParentLines.second = {};
	Line::layer = 0;
	API_Coord z, z1;
	z.x = 0;
	z.y = 0;
	z1.x = 1;
	z1.y = 0;

	beg = z;
	end = z1;
	if (z.x != z1.x && z.y != z1.y) {
		m = ((z.y) - (z1.y)) / ((z.x) - (z1.x));
		c = z1.y - m * z1.x;
	}
	else
	{
		if (z.x == z1.x)
		{
			ParallelToY = true;
			m = 0;
			c = 0;
		}
		else
		{
			ParallelToX = true;
			m = 0;
			c = 0;
		}

	}
}

Line::Line(API_Coord a, double newM) {
	Line::guid = {};
	Line::PenInd = 0;
	Line::layer = 0;
	beg = a;
	m = newM;
	c = beg.y - m * beg.x;
	end.x = 10000;
	end.y = end.x * m + c;
	if (m == 0)
		ParallelToX = true;
	if (m == INFINITY)
		ParallelToY = true;
}

bool Line::IsPointOnLine(API_Coord point, double sensitivity)
{
	if (ParallelToX)
	{
		if (abs(point.y - beg.y) < sensitivity)
			return true;
		else return false;
	}
	if (ParallelToY)
	{
		if (abs(point.x - beg.x) < sensitivity)
			return true;
		else return false;
	}
	if (abs(point.y - (m * point.x + c)) < sensitivity)
		return true;
	else return false;
}

bool Line::IsParallel(Line lineA, Line lineB)
{
	if (lineA.ParallelToX && lineB.ParallelToX)
		return true;
	if (lineA.ParallelToY && lineB.ParallelToY)
		return true;
	if (abs(lineA.m - lineB.m) < 0.01 && lineA.m != 0)
		return true;
	else return false;
}
bool Line::IsPerp(Line lineA, Line lineB)
{
	if (lineA.ParallelToX && lineB.ParallelToY)
		return true;
	if (lineA.ParallelToY && lineB.ParallelToX)
		return true;
	if (lineA.m * lineB.m == -1.0)
		return true;
	else return false;
}

API_Coord Line::LineMeetingPoint(Line lineA, Line lineB) {
	//assuming they are not parralel.
	API_Coord MeetP;
	MeetP.x = INFINITY, MeetP.y = INFINITY;
	if ((lineA.ParallelToX == true && lineB.ParallelToX == true) || (lineA.ParallelToY == true && lineB.ParallelToY == true))
		return MeetP;

	if (lineA.ParallelToX)
	{
		if (lineB.ParallelToY) {
			MeetP.y = lineA.beg.y;
			MeetP.x = lineB.beg.x;
			return MeetP;
		}
		else {
			MeetP.y = lineA.beg.y;
			MeetP.x = (MeetP.y - lineB.c) / lineB.m;
			return MeetP;
		}
	}
	if (lineB.ParallelToX)
	{
		if (lineA.ParallelToY) {
			MeetP.y = lineB.beg.y;
			MeetP.x = lineA.beg.x;
			return MeetP;
		}
		else {
			MeetP.y = lineB.beg.y;
			MeetP.x = (MeetP.y - lineA.c) / lineA.m;
			return MeetP;
		}
	}

	if (lineA.ParallelToY)
	{
		if (lineB.ParallelToX) {
			MeetP.x = lineA.beg.x;
			MeetP.y = lineB.beg.y;
			return MeetP;
		}
		else {
			MeetP.x = lineA.beg.x;
			MeetP.y = lineB.m * MeetP.x + lineB.c;
			return MeetP;
		}
	}
	if (lineB.ParallelToY)
	{
		if (lineA.ParallelToX) {
			MeetP.x = lineB.beg.x;
			MeetP.y = lineA.beg.y;
			return MeetP;
		}
		else {
			MeetP.x = lineB.beg.x;
			MeetP.y = lineA.m * MeetP.x + lineA.c;
			return MeetP;
		}
	}

	MeetP.x = (lineB.c - lineA.c) / (lineA.m - lineB.m);
	MeetP.y = lineA.m * MeetP.x + lineA.c;
	return MeetP;

}

bool Line::IsPointOnRealPartOfLine(API_Coord point, API_Coord startL, API_Coord endL) {
	Line line1(startL, endL);
	if (!line1.IsPointOnLine(point, 0.3))
		return false;
	if (line1.ParallelToX) {
		if ((point.x >= std::min(startL.x, endL.x)) && ((point.x <= std::max(startL.x, endL.x))))
			return true;
		else return false;
	}
	if (line1.ParallelToY) {
		if ((point.y >= std::min(startL.y, endL.y)) && ((point.y <= std::max(startL.y, endL.y))))
			return true;
		else return false;
	}
	else {
		if ((point.x >= std::min(startL.x, endL.x)) && ((point.x <= std::max(startL.x, endL.x))))
			return true;
		else return false;
	}
	//normal line


}

Line Line::CreateTangentByDeg(API_Coord p, double deg) {
	if (deg == 90 || deg == 270) {
		API_Coord b = p;
		b.y += 1;
		return Line(b, p);
	}
	else {
		double m = tan((deg * PI) / 180.0);
		return Line(p, m);
	}
}

double Line::distanceBetweenPoints(API_Coord a, API_Coord b) {
	return sqrt(((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y)));
}

double Line::distanceBetweenLines(Line lineA, Line lineB) {

	if (!Line::IsParallel(lineA, lineB))
		return 0;

	if (lineA.ParallelToX && lineB.ParallelToX)
		return abs(lineA.beg.y - lineB.beg.y);

	if (lineA.ParallelToY && lineB.ParallelToY)
		return abs(lineA.beg.x - lineB.beg.x);


	else {
		API_Coord a;
		a.x = 0;
		a.y = lineA.m * a.x + lineA.c;
		Line Perp = Line::Line(a, (-1 / lineA.m));
		API_Coord b = Line::LineMeetingPoint(lineB, Perp);
		return Line::distanceBetweenPoints(a, b);
	}
}

double Line::PointLineDistance(API_Coord point, Line line)
{
	if (line.ParallelToX)
	{
		return(abs(line.beg.y - point.y));
	}
	if (line.ParallelToY)
	{
		return(abs(line.beg.x - point.x));
	}
	return abs((line.m * point.x - point.y + line.c) / (sqrt(line.m * line.m + 1)));

}

double Line::GetAngleFromArc(API_Element arc) {
	double smallAng = std::min(arc.arc.begAng, arc.arc.endAng);
	double LargeAng = std::max(arc.arc.begAng, arc.arc.endAng);
	double deg = (LargeAng - smallAng) * 180.0 / PI;
	return abs(deg);

}

double Line::GetRealLength(Line lineA) {
	return Line::distanceBetweenPoints(lineA.beg, lineA.end);
}

bool Line::AreLinesConverging(Line lineA, Line lineB, double sensitivity) {
	if (!Line::IsParallel(lineA, lineB))
		return false;
	if (lineA.ParallelToX && lineB.ParallelToX) {
		if (abs(lineA.beg.y - lineB.beg.y) < sensitivity || abs(lineA.end.y - lineB.end.y) < sensitivity)
			return true;
		else return false;
	}

	if (lineA.ParallelToY && lineB.ParallelToY) {
		if (abs(lineA.beg.x - lineB.beg.x) < sensitivity || abs(lineA.end.x - lineB.end.x) < sensitivity)
			return true;
		else return false;
	}

	if (abs(lineA.c - lineB.c) < sensitivity)
		return true;
	return false;
}

API_Coord Line::GetArcBegCoord(API_Element arc) {

	API_Coord BegC;
	BegC.x = INFINITY;
	BegC.y = INFINITY;
	GS::Array<API_ElementHotspot>  hotspotArray;
	API_Guid    guid = arc.header.guid;    /* GUID of an element placed on the floorplan */
	GSErrCode err = ACAPI_Element_GetHotspots(guid, &hotspotArray);
	if (err == NoError && !hotspotArray.IsEmpty()) {
		API_Coord3D coord;
		coord = hotspotArray.Get(1).second;
		BegC.x = coord.x;
		BegC.y = coord.y;
	}
	return BegC;

}

API_Coord Line::GetArcEndCoord(API_Element arc) {

	API_Coord BegC;
	BegC.x = INFINITY;
	BegC.y = INFINITY;
	GS::Array<API_ElementHotspot>  hotspotArray;
	API_Guid    guid = arc.header.guid;    /* GUID of an element placed on the floorplan */
	GSErrCode err = ACAPI_Element_GetHotspots(guid, &hotspotArray);
	if (err == NoError && !hotspotArray.IsEmpty()) {
		API_Coord3D coord;
		coord = hotspotArray.Get(2).second;
		BegC.x = coord.x;
		BegC.y = coord.y;
	}
	return BegC;

}

API_Coord Line::GetArcCenterCoord(API_Element arc) {

	API_Coord BegC;
	BegC.x = INFINITY;
	BegC.y = INFINITY;
	GS::Array<API_ElementHotspot>  hotspotArray;
	API_Guid    guid = arc.header.guid;    /* GUID of an element placed on the floorplan */
	GSErrCode err = ACAPI_Element_GetHotspots(guid, &hotspotArray);
	if (err == NoError && !hotspotArray.IsEmpty()) {
		API_Coord3D coord;
		coord = hotspotArray.Get(0).second;
		BegC.x = coord.x;
		BegC.y = coord.y;
	}
	return BegC;

}


API_Coord Line::GetClosesetPointOnline(Line line, API_Coord point) {
	API_Coord c;
	c.x = 0, c.y = 0;
	if (!line.ParallelToX && !line.ParallelToY)
	{
		double newM = -1 / line.m;
		Line Tangent = Line(point, newM);
		c = Line::LineMeetingPoint(line, Tangent);
		return c;
	}

	if (line.ParallelToX)
	{
		c.x = point.x;
		c.y = line.beg.y;
		return c;
	}
	if (line.ParallelToY)
	{
		c.y = point.y;
		c.x = line.beg.x;
		return c;
	}
	return c;
}

bool Line::AreLinesInSameDirection(Line lineA, Line lineB) {
	double difA = 0;
	double difB = 0;
	if (lineA.ParallelToX && lineB.ParallelToX)
	{
		double difA = 0;
		double difB = 0;
		difA = lineA.end.x - lineA.beg.x;
		difB = lineB.end.x - lineB.beg.x;
		if (difA > 0 && difB > 0)
			return true;
		if (difA < 0 && difB < 0)
			return true;
		return false;
	}

	if (lineA.ParallelToY && lineB.ParallelToY)
	{
		double difA = 0;
		double difB = 0;
		difA = lineA.end.y - lineA.beg.y;
		difB = lineB.end.y - lineB.beg.y;
		if (difA > 0 && difB > 0)
			return true;
		if (difA < 0 && difB < 0)
			return true;
		return false;
	}


	difA = lineA.end.x - lineA.beg.x;
	difB = lineB.end.x - lineB.beg.x;
	if (difA > 0 && difB > 0)
		return true;
	if (difA < 0 && difB < 0)
		return true;
	return false;

}

bool Line::IsLineInPositiveDir(Line lineA) {
	double difA = 0;

	if (lineA.ParallelToX)
	{
		double difA = 0;
		difA = lineA.end.x - lineA.beg.x;
		if (difA > 0)
			return true;
		return false;
	}

	if (lineA.ParallelToY)
	{
		double difA = 0;
		difA = lineA.end.y - lineA.beg.y;
		if (difA > 0)
			return true;
		return false;
	}


	difA = lineA.end.x - lineA.beg.x;
	if (difA > 0)
		return true;
	return false;


}


bool Line::AreLinesIdentical(Line lineA, Line lineB) {
	if (abs(lineA.beg.x - lineB.beg.x) < 0.01 && abs(lineA.beg.y - lineB.beg.y) < 0.01)
		if (abs(lineA.end.x - lineB.end.x) < 0.01 && abs(lineA.end.y - lineB.end.y) < 0.01)
			return true;
	if (abs(lineA.beg.x - lineB.end.x) < 0.01 && abs(lineA.beg.y - lineB.end.y) < 0.01)
		if (abs(lineA.end.x - lineB.beg.x) < 0.01 && abs(lineA.end.y - lineB.beg.y) < 0.01)
			return true;
	return false;
}
GS::Array<Line> Line::GetLinesFromPoly(API_Element poly) {
	GS::Array<Line> PolyLines;

	API_ElementMemo memo;
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	ACAPI_Element_GetMemo(poly.header.guid, &memo);
	Int32 n = BMhGetSize((GSHandle)memo.coords) / Sizeof32(API_Coord);
	for (Int32 k = 1; k < n; k++) { // for each coord
		if (k + 1 == n) {
			PolyLines.Push(Line((*memo.coords)[n - 1], (*memo.coords)[1], poly));
		}
		else
		{
			PolyLines.Push(Line((*memo.coords)[k], (*memo.coords)[k + 1], poly));
		}
	}

	for (GS::USize i = 0; i < PolyLines.GetSize(); i++)
	{
		for (GS::USize j = 0; j < PolyLines.GetSize();)
		{
			if (i != j && Line::AreLinesIdentical(PolyLines.Get(i), PolyLines.Get(j)))
				PolyLines.Delete(j);
			else j++;
		}
	}
	return PolyLines;

}

double Line::GetAngleBetweenLines(Line lineA, Line lineB)
{
	if(Line::IsParallel(lineA, lineB) || Line::AreLinesConverging(lineA, lineB, 0.4))
		return 0;
	if (Line::IsPerp(lineA, lineB))
		return 90;

	if (lineA.ParallelToX || lineB.ParallelToX)
	{
		if (lineA.ParallelToX)
			return ((atan(lineB.m) * 180 / PI));
		else
			return ((atan(lineA.m) * 180) / PI);
	}
	
	if (lineA.ParallelToY || lineB.ParallelToY)
	{
		if (lineA.ParallelToY)
			return (std::max(((atan(lineB.m) * 180) / PI), 90.0) - std::min(((atan(lineB.m) * 180) / PI), 90.0));
		else
			return (std::max((atan(lineA.m) * 180 / PI), 90.0) - std::min((atan(lineA.m) * 180 / PI), 90.0));
	}

	return (std::max((atan(lineA.m) * 180 / PI), (atan(lineB.m) * 180 / PI)) - std::min((atan(lineA.m) * 180 / PI), (atan(lineB.m) * 180 / PI)));
}


