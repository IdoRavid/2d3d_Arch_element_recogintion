#include "TmrTech.h"
#include "APIEnvir.h" //Importing API environment
#include <DG.h> //Importing Dialogs library.
#pragma comment (lib, "Gdiplus.lib")
#include "APIEnvir.h" //Importing API environment
#include <DG.h> //Importing Dialogs library.
#include "TmrTech.h"
#include    "Queue.hpp"
#include	<stdio.h>
#include	<string.h>
#include	<sstream>
#include    <iostream>
#include    <fstream>
#include	"ACAPinc.h"					// also includes APIdefs.h
#include	"APIdefs_Database.h"
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
#include "OBJ_Exporter.h"
#include <ACAPinc.h>
#include <locale>
#include <codecvt>
#include	<DGFolderDialog.hpp>
#include <cstring>
#include <string>
#include "SavingFunctions.h"
#include "TheRenderer.h"
#include <DGUtility.hpp>
using namespace Gdiplus;
using namespace DG;


// ---------------------------------- Includes ---------------------------------
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

#include	"APICommon.h"
#include	"UniString.hpp"
#include	<Windows.h>
#include <Color.hpp>
#include	"Array.hpp"

#include <cstring>
#include <string>
using namespace std;


// -------------------------------------- Technical Functions ---------------------

void TmrTech::Do_DeselectAll(void)
{
	GSErrCode		err; //Creating err to get the result of our action
	err = ACAPI_Element_DeselectAll(); //Choosing nothing - which is to "deselect"
	if (err != NoError)
		ErrorBeep("ACAPI_Element_Select", err); //Returns error if we had one

	return;
}		// Do_DeselectAll
//Deselect everything

void TmrTech::Do_SelectAll()
{
	GSErrCode err;
	GS::Array<API_Guid> elemList; //List of elements
	API_Neig neig;

	err = ACAPI_Element_GetElemList(API_ZombieElemID, &elemList, APIFilt_OnVisLayer | APIFilt_OnActFloor);
	if (err != NoError)
	{
		ErrorBeep("ACAPI_GetElemList", err);
		return;
	}
	DBPrintf("number of elements on the active floor: %ld", elemList.GetSize());

	GS::Array<API_Neig> neigs;

	for (UInt32 i = 0; i < elemList.GetSize(); i++)
	{
		API_Neig neig(elemList[i]);
		neigs.Push(neig);
	}

	Do_DeselectAll();

	err = ACAPI_Element_Select(neigs, true); //Selecting all elements
	if (err != NoError)
	{
		ErrorBeep("ACAPI_Element_Select", err);
		return;
	}
	return;
}
// Selecting all elements

string TmrTech::ReplaceInString(string origin, string toReplace, string newStr) {//Changes the first occurance of the toReplace string inside the origin
	size_t index = 0;
	if (origin.find(toReplace, index) < origin.npos) {
		index = origin.find(toReplace, index);
		return origin.replace(index, toReplace.length(), newStr);
	}
	else
		return origin;
}

string TmrTech::LowLetter(string ourString) //Replaces all the Capital Letters to Low letters
{
	int counter = 0;

	for each (char c in ourString)
	{
		if (isalpha(c) != 0 && isupper(c) != 0) //If the string is upper, we'll low it:
		{
			int one = tolower(c);
			ourString[counter] = (char)one;
		}
		counter++; //Adding to the counter to know at what index we are
	}
	return ourString; //returning our low-lettered string.
}

string TmrTech::AddSlashes(string ourString) //add another backslash to a backslash in the string 
{
	size_t found = ourString.find("\\"); //finding the slashes
	while (found != string::npos)
	{
		ourString.replace(found, 0, "\\");
		found = ourString.find("\\", found + 2); //Find again, but lgnore the first 2 ones (because we fixed them)
	}
	return ourString; //returning our double-slashed string.
}

GS::UniString  TmrTech::StringIntoUnistring(string ourString) //exchange Unichar at char, and therfore converts the string into Unistring.
{
	return GS::UniString(ourString.c_str());
}

void TmrTech::errtoName(GSErrCode err)//prints the err on the screen
{
	if (err != NoError)
	{//If there's an error
		ACAPI_WriteReport("the err is %s", true, ErrID_To_Name(err)); //Print it
	}
}

short TmrTech::GetFirstNumInString(GS::UniString st) {//Returning the first full num in the given string (Examples: ״קומה ק+2״ will return 2. ״33 אנשים״ will return 33.
	GS::UniString firstNum = "";
	bool inANum = false;//If we are finding a num we want to leave the loop when it's over
	for (USize i = 0; i < st.GetLength(); i++) {
		if (st[i].IsDigit()) {
			firstNum += st[i];
			inANum = true;
		}
		else if (inANum)
			break;
	}
	if (firstNum.GetLength() > 0) {
		std::string numString = firstNum.ToCStr().Get();
		int num = stoi(numString);
		short sNum = static_cast<short>(num);
		return sNum;
	}
	else
		return -1;//No number in the string
}

// -------------------------------- File Location

GS::UniString TmrTech::CurrentFilePath(bool withProjectName) { //Returning the path of the folder which the current file is on/full filename
	API_ProjectInfo pj; //Creating a projectInfo var
	BNZeroMemory(&pj, sizeof(API_ProjectInfo)); //Clearing memory for it
	ACAPI_Environment(APIEnv_ProjectID, &pj, 0); //Putting the info in it
	GS::UniString p; //The path of the file
	string path = "";
	if (pj.location != nullptr) {
		IO::Location
			loc = *pj.location;
		IO::Name projectName;
		loc.GetLastLocalName(&projectName);
		loc.ToPath(&p);
		if (withProjectName == false)
			p.ReplaceAll(projectName.ToString(), "");
	}
	else {
		ACAPI_WriteReport("The project isn't saved. it was saved to C:\\Temp\\Tik", true);
		if (withProjectName)
			p = "C:\\Temp\\Tik\\Main.pln";
		else
			p = "C:\\Temp\\Tik\\";
		SavingFunctions().Do_SavePLN("C:\\Temp\\Tik\\Main.pln");
	}
	return p;
}

void TmrTech::moveFile(string from, string to) {
	ifstream in(from);
	ofstream out(to);
	out << in.rdbuf();
	out.close();
	in.close();
}

// ------------------------ Select Invert -------------------------------

void TmrTech::Do_SelectInvert()
{//select all elements are oposite to your choice
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::Array<API_Neig> selNeigs;

	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
	if (err != NoError) {
		ErrorBeep("ACAPI_Selection_Get", err);
		return;
	}
	BMKillHandle((GSHandle*)& selectionInfo.marquee.coords);

	TmrTech().Do_DeselectAll();
	TmrTech().Do_SelectAll();
	if (selectionInfo.typeID != API_SelEmpty)
	{
		
		err = ACAPI_Element_Select(selNeigs, false);
		if (err != NoError) {
			ErrorBeep("ACAPI_Element_Select", err);
			return;
		}
	}
	//BMKillHandle((GSHandle*)& selNeigs);
	return;
}
//s
void TmrTech::Do_SelectInvertSameID()
{//Select all the oposite elements of your choice, but selects only the type you chose.
	GS::Array<API_Guid> elemList;
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	API_Element        element;
	GS::Array<API_Neig> selNeigs;


	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);     //information about current selection.
	if (err != NoError)
	{
		ErrorBeep("ACAPI_Selection_Get", err);
		return;
	}

	if (selectionInfo.typeID != API_SelEmpty)
	{
		 // collect indexes of selected elements 

		element.header.typeID = Neig_To_ElemID((selNeigs)[0].neigID); // get the value of the chosen type ID

		err = ACAPI_Element_GetElemList(element.header.typeID, &elemList, APIFilt_OnActFloor);      //gets all elements of specified ID
		if (err != NoError)
		{
			ErrorBeep("ACAPI_Element_GetElemList", err);
			return;
		}
		const UIndex elemAmount = (elemList.GetSize());          //number of the same ID  of chosen element

																// The following line defines a pointer to an array of type API_Neig with length elemAmount.
		GS::Array<API_Neig> neig;

		TmrTech().Do_DeselectAll();

		for (GS::UIndex ii = 0; ii < elemAmount && err == NoError; ++ii)
		{
			API_Neig neigy(elemList[ii]);
			neig.Push(neigy);    //handles all the elements

		}

		err = ACAPI_Element_Select(neig, true);     //selects all elements with the same type as chosen before
		if (err != NoError)
		{
			ErrorBeep("ACAPI_Element_Select", err);
			return;
		}

		err = ACAPI_Element_Select(selNeigs,  false);     //removes the inverted elements selected before.
		if (err != NoError)
		{
			ErrorBeep("ACAPI_Element_Select", err);
			return;
		}

		//BMKillHandle((GSHandle*)& neig); //realeases allocated memory

	}
	//BMKillHandle((GSHandle*)& selNeigs);
	//BMKillHandle((GSHandle*)& selectionInfo.marquee.coords);

	return;
}

void TmrTech::CreateNewMaterial(GS::UniString name, string LocationString ) {

	API_Attribute      newmaterial;
	API_Attribute      prototypemat;
	API_Texture        newtexture;
	API_AttributeIndex  ltypeIndex;
	GSErrCode          err;

	 IO::Location            fileLoc(LocationString.c_str());
	 BNZeroMemory(&prototypemat, sizeof(API_Attribute));
	 prototypemat.header.index = 258;
	 prototypemat.header.typeID = API_MaterialID;
	 ACAPI_Attribute_Get(&prototypemat);


	BNZeroMemory(&newmaterial, sizeof(API_Attribute));
	newtexture = prototypemat.material.texture;
	newmaterial.header.typeID = API_MaterialID; //def type
	newmaterial.header.uniStringNamePtr = &name; //insert material name
	newtexture.fileLoc = &fileLoc; //insert newtexture's location
	
	 API_RGBColor white;
	 white.f_blue = 1.0;
	 white.f_green = 1.0;
	 white.f_red = 1.0;
	 newmaterial.material.mtype = APIMater_GeneralID;
	 newmaterial.material.surfaceRGB = white;
	newmaterial.material.emissionRGB = white;
	newmaterial.material.specularRGB = white;
	newmaterial.material.diffusePc = 100;
	newmaterial.material.texture = newtexture; //apply new texture to new material*/


	err = ACAPI_Attribute_Create(&newmaterial, nullptr); //create new material
	ltypeIndex = newmaterial.header.index;
	
}

void TmrTech::Quit()
{

	GSErrCode	err;

	err = ACAPI_Automate(APIDo_QuitID, (void*)(Int32)1234);
	if (err != NoError)
		ACAPI_WriteReport("Error in APIDo_CloseID: %s", true, ErrID_To_Name(err));

	return;
}

void TmrTech::DoSaveU3D()
{
	//first we want to get each floor's name and id
	GSErrCode err;
	string projectPath = "C:\\Users\\T81971451\\Desktop\\Shputzi.txt";
	API_Element         element1;

	GSErrCode           err1 = NoError;
	API_StoryInfo storyInfo;

	std::string floornamestring;

	err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
	UInt32 floornum = storyInfo.lastStory - storyInfo.firstStory; // HERE WE GET THE NUMBER OF FLOORS
	ofstream file(projectPath);
	for (UInt32 i = 0; i < floornum + 1; i++)

	{
		API_StoryCmdType storyCmd; //changing active floor to the relevant one
		storyCmd.action = APIStory_GoTo;
		storyCmd.index = (short)i;
		ACAPI_Environment(APIEnv_ChangeStorySettingsID, &storyCmd);
		err = ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr);
		if (i == 0)
		{
			file.open(projectPath);
		}
		else
		{
			file.open(projectPath, std::ios::app);
		}

		GS::UniString a = GS::UniString((*storyInfo.data)[i].uName).ToCStr().Get();
		file << "\n";
		file << "\n";
		file << "Floor Name: ";
		file << a.ToCStr().Get();
		file << "\n";
		file << "\n";
		file.close();

		GS::Array<API_Guid> elemList;
		ACAPI_Element_GetElemList(API_ZombieElemID, &elemList);

		for (UInt32 j = 0; j < elemList.GetSize(); j++) //מעבר על כל האובייקטים 
		{
			BNZeroMemory(&element1, sizeof(API_Element)); //free up memory
			element1.header.guid = elemList[j]; //enter the wall Guid to element1
			err1 = ACAPI_Element_Get(&element1);
			if (err1 != NoError)
			{
				ACAPI_WriteReport("err: %s:", true, ErrID_To_Name(err1));
			}
			if (ACAPI_Element_Filter(element1.header.guid, APIFilt_In3D, APIVarId_Generic, APINULLGuid) && element1.header.floorInd == (*storyInfo.data)[i].index)
			{
				file.open(projectPath, std::ios::app);
				file << GS::UniString(APIGuidToString(element1.header.guid)).ToCStr().Get();
				file << "\n";
				file.close();
			}
		}
	}
	BMKillHandle((GSHandle*)& storyInfo.data);//Disposing storyInfo.data because we used it.
}

void TmrTech::ShowSelectionGUID() {

	API_Neig selNeigs;
	if (ClickAnElem("Click an elem to select/deselect", API_ZombieElemID, &selNeigs))
		ACAPI_WriteReport(APIGuid2GSGuid(selNeigs.guid).ToUniString(), true);
}

void TmrTech::GetSelection() {
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	GS::Array<API_Neig>  selNeigs;

	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
	BMKillHandle((GSHandle*)& selectionInfo.marquee.coords);
	if (err == APIERR_NOSEL)
		err = NoError;

	API_Guid SelGuid = selNeigs.GetFirst().guid;
	API_Element element;
	

	BNZeroMemory(&element, sizeof(API_Element));
	element.header.guid = SelGuid;
	ACAPI_Element_Get(&element);
	element = element;

	API_ElementMemo CurMemo;
	BNZeroMemory(&CurMemo, sizeof(API_ElementMemo));
	ACAPI_Element_GetMemo(SelGuid, &CurMemo);
	GS::Array<API_Coord> coArr;
	Int32 n = BMhGetSize((GSHandle)CurMemo.coords) / Sizeof32(API_Coord);
	for (Int32 i = 1; i < n; i++)
	{
		coArr.Push((*CurMemo.coords)[i]);
	}
	coArr = coArr;


}

void TmrTech::ChangeCurrentDatabase(API_WindowTypeID id)
{
	API_DatabaseInfo    databasePars;
	BNZeroMemory(&databasePars, sizeof(API_DatabaseInfo));
	databasePars.typeID = id;
	ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &databasePars);
}

