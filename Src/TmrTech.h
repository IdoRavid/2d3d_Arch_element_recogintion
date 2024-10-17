#pragma once

#define	_SmartTik_TRANSL_
#include <DG.h> //Importing Dialogs library.
#include    <fstream>
#include "TmrTech.h"
#include "APIEnvir.h" //Importing API environment
#include <DG.h> //Importing Dialogs library.
#pragma comment (lib, "Gdiplus.lib")
#include "TmrTech.h"
#include    "Queue.hpp"
#include	<stdio.h>
#include	<string.h>
#include	<sstream>
#include    <iostream>
#include    <fstream>
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APIdefs_Database.h"
#include "APIdefs_Elements.h "
#include "APIdefs_Automate.h"
#include	<math.h>
#include	<string.h>
#include	"HashSet.hpp"
#include	<DGModule.hpp>
#include <direct.h>
#include	"APICommon.h"
#include	"UniString.hpp"
#include	<Windows.h>
#include <Unknwn.h>
#include <Color.hpp>
#include	"Array.hpp"
#include <objidl.h>
#include <gdiplus.h>
#include "DGListView.hpp"
#include <DGListView.hpp>
#include <DGItem.hpp>
#include <DGItemProperty.hpp>
//#pragma comment (lib, "DG.lib")
#include <cstring>
#include <string>
#include <DGUtility.hpp>
using namespace Gdiplus;
using namespace DG;
using namespace std;

class TmrTech
{
public:
	static void	Do_DeselectAll(void);//Deselect everything

	void Do_SelectAll();//Select all elements

	static string ReplaceInString(string origin, string toReplace, string newStr);

	static string LowLetter(string ourString); //

	string AddSlashes(string ourString);//Add Slashaes to Path

	static GS::UniString  StringIntoUnistring(string ourString);

	static void errtoName(GSErrCode err);//error to Name + print the error

	static GS::UniString CurrentFilePath(bool withProjectName);//Get Current AC project file path

	static void Do_SelectInvertSameID();

	static void Do_SelectInvert();
	
	static void TmrTech::CreateNewMaterial(GS::UniString name, string LocationString);

	static void  TmrTech::Quit();

	static void TmrTech::DoSaveU3D();

	static void ShowSelectionGUID();

	static void GetSelection();

	static void moveFile(string from, string to);

	static short GetFirstNumInString(GS::UniString st);

	static void ChangeCurrentDatabase(API_WindowTypeID id);
};