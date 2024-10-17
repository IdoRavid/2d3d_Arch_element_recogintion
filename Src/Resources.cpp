// *****************************************************************************
// Source code for the Navigator Test Add-On
// API Development Kit 23; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#include "Resources.hpp"

#define	 ACExtension
#include "ACAPinc.h"

GS::UniString GetLocStr (NavigatorAPILocalisedStringIds stringId)
{
	GS::UniString string = RSGetIndString (NAVIGATOR_TEST_RESID_LOCSTRINGS, stringId, ACAPI_GetOwnResModule ());
	return string;
}