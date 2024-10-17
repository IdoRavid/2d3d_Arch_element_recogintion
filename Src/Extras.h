#pragma once
#include "Extras.h"
#include "APIEnvir.h"

#define	_Extras_TRANSL_


// ---------------------------------- Includes ---------------------------------

#include	<string.h>
#include <iostream>
#include <string>
#include	"GSRoot.hpp"
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APICommon.h"
class Extras
{
public:
static	void Do_CutFloorUnderStairs();
static  void Do_CreateSlabHole(API_Guid SlabG, API_Guid stairG);
static void Do_CreateSlabHoleEdit(API_Guid SlabG, API_Guid stairG);
static void ReClassifyColumns();
static void OrganizeLayers();
static void Do_CreateCircle(const API_Coord& point);
static bool IsWallExternal(API_Guid guid);
static void GetExternalWalls();
static void GetInternalWall();
static void IsSelectionExternalWall();
};

