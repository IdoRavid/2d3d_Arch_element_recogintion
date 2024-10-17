#pragma once
#include "GeoAnalysis.h"
#include "APIEnvir.h"



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
class Line
{
public:
	double m = 0;
	double c = 0;
	bool ParallelToX = false;
	bool ParallelToY = false;
	API_Coord beg, end;
	API_Guid guid;
	short PenInd;
	API_AttributeIndex layer;
	std::pair<API_Guid, API_Guid> ParentLines;
	Line(API_Guid Lineguid);
	Line(API_Coord a, API_Coord b, Line ParentA, Line ParentB);
	Line(API_Coord a, API_Coord b);
	Line(API_Coord a, API_Coord b, API_Element poly);
	 Line();
	 Line(API_Coord a, double newM);
	 bool IsPointOnLine(API_Coord point, double sensitivity);
	 static bool IsParallel(Line lineA, Line lineB);
	 static bool IsPerp(Line lineA, Line lineB);
	 static API_Coord LineMeetingPoint(Line lineA, Line lineB);
	 static bool IsPointOnRealPartOfLine(API_Coord point, API_Coord startL, API_Coord endL);
	 static Line CreateTangentByDeg(API_Coord p, double deg);
	 static double distanceBetweenPoints(API_Coord a, API_Coord b);
	 static double distanceBetweenLines(Line lineA, Line lineB);
	 static double PointLineDistance(API_Coord point, Line line);
	 static double GetAngleFromArc(API_Element arc);
	 static double GetRealLength(Line lineA);
	 static bool AreLinesConverging(Line lineA, Line lineB, double sensitivity);
	 static API_Coord GetArcBegCoord(API_Element arc);
	 static API_Coord GetArcEndCoord(API_Element arc);
	 static API_Coord GetArcCenterCoord(API_Element arc);
	 static API_Coord GetClosesetPointOnline(Line line, API_Coord point);
	 static bool AreLinesInSameDirection(Line lineA, Line lineB);
	 static bool IsLineInPositiveDir(Line lineA);
	 static  bool Line::AreLinesIdentical(Line lineA, Line lineB);
	 static GS::Array<Line> GetLinesFromPoly(API_Element poly);
	 static double GetAngleBetweenLines(Line lineA, Line lineB);
	 static bool AreLinesOnSameLine(Line lineA, Line lineB);
};

