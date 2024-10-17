#include "ColRec.h"

struct ColRec::column {
	API_Coord point;
	double width, length;
	API_Guid guid;
};

void ColRec::CreateColumnsFromDrawingPolys() {

	GS::Array <column>	ColPolysArr = ColRec::GetColumnPolys();
	GS::Array <column>	ColFillsArr = ColRec::GetColumnFills();

	for (GS::USize i = 0; i < ColFillsArr.GetSize(); i++)
	{
		ColPolysArr.Push(ColFillsArr.Get(i));

	}

	ColPolysArr = ColRec::DeleteDupColumns(ColPolysArr);
	
	for (GS::USize i = 0; i < ColPolysArr.GetSize(); i++)
	{
		ColRec::CreateColumnElement(ColPolysArr.Get(i));
	}
}

void ColRec::CreateColumnElement(column col) {
	API_Element element;
	API_ElementMemo memo;
	BNClear(element);
	BNClear(memo);

	element.header.typeID = API_ColumnID;
	GSErrCode err = ACAPI_Element_GetDefaults(&element, &memo);

	if (err != NoError) {
		ACAPI_DisposeElemMemoHdls(&memo);
		return;
	}
	memo.columnSegments->assemblySegmentData.nominalWidth = col.width;
	memo.columnSegments->assemblySegmentData.nominalHeight = col.length;
	memo.columnSegments->assemblySegmentData.isWidthAndHeightLinked = false;


	element.column.origoPos = col.point;
	element.column.coreAnchor = 4;

	err = ACAPI_CallUndoableCommand("Create Column",
		[&]() -> GSErrCode {
			return err = ACAPI_Element_Create(&element, &memo);
		});
	if (err != NoError) {
		ACAPI_WriteReport("ACAPI_Element_Create (Column) has failed", false, err);
	}
	ACAPI_DisposeElemMemoHdls(&memo);
}



GS::Array<ColRec::column> ColRec::GetColumnPolys()
{
	GS::Array<ColRec::column> colArr;
	for (GS::USize i = 0; i < RecHandler::UPoly.GetSize(); )
	{//for each Unidentified Line
		API_Element CurPoly;
		API_ElementMemo CurMemo;
		BNZeroMemory(&CurPoly, sizeof(API_Element));
		CurPoly.header.guid = RecHandler::UPoly.Get(i);
		ACAPI_Element_Get(&CurPoly);
		ACAPI_Element_GetMemo(CurPoly.header.guid, &CurMemo);
		bool deleted = false;
		if (CurPoly.polyLine.poly.nCoords == 5) {
			if (Line::IsPerp(Line((*CurMemo.coords)[1], (*CurMemo.coords)[2]), Line((*CurMemo.coords)[2], (*CurMemo.coords)[3])))
			{
				if ((RecHandler::layerMap[GS::UniString("Columns")].GetSize() == 0) || (RecHandler::layerMap[GS::UniString("Columns")].Contains(CurPoly.header.layer) || (1 == CurPoly.header.layer)))
				{
					column col;
					Line A = Line((*CurMemo.coords)[1], (*CurMemo.coords)[2]);
					Line B = Line((*CurMemo.coords)[2], (*CurMemo.coords)[3]);
					if (A.ParallelToX)
					{
						col.length = Line::distanceBetweenPoints(B.beg, B.end);
						col.width = Line::distanceBetweenPoints(A.beg, A.end);
					}
					else
					{
						col.length = Line::distanceBetweenPoints(A.beg, A.end);
						col.width = Line::distanceBetweenPoints(B.beg, B.end);
					}
					col.guid = CurPoly.header.guid;
					GS::Array<API_Coord> coArr;
					Int32 n = BMhGetSize((GSHandle)CurMemo.coords) / Sizeof32(API_Coord);
					for (Int32 i = 1; i < n - 1; i++)
					{
						coArr.Push((*CurMemo.coords)[i]);
					}
					if (col.width < 8 && col.length < 8) {
						col.point = RoomRec::GetArrMidPoint(coArr);
						colArr.Push(col);
						RecHandler::UPoly.Delete(i);
						deleted = true;
					}
					
				}

			}
			
		}
		if(!deleted) 
			i++;

	}
	return colArr;
}

GS::Array<ColRec::column> ColRec::GetColumnFills()
{
	GS::Array<ColRec::column> colArr;
	for (GS::USize i = 0; i < RecHandler::UFill.GetSize(); )
	{//for each Unidentified Line
		API_Element CurFill;
		API_ElementMemo CurMemo;
		BNZeroMemory(&CurFill, sizeof(API_Element));
		CurFill.header.guid = RecHandler::UFill.Get(i);
		ACAPI_Element_Get(&CurFill);
		ACAPI_Element_GetMemo(CurFill.header.guid, &CurMemo);
		bool deleted = false;
		if (CurFill.hatch.poly.nCoords == 5) {
			if (Line::IsPerp(Line((*CurMemo.coords)[1], (*CurMemo.coords)[2]), Line((*CurMemo.coords)[2], (*CurMemo.coords)[3])))
			{
				if ((RecHandler::layerMap[GS::UniString("Columns")].GetSize() == 0) || (RecHandler::layerMap[GS::UniString("Columns")].Contains(CurFill.header.layer) || (1 == CurFill.header.layer)))
				{
					column col;
					Line A = Line((*CurMemo.coords)[1], (*CurMemo.coords)[2]);
					Line B = Line((*CurMemo.coords)[2], (*CurMemo.coords)[3]);
					if (A.ParallelToX)
					{
						col.length = Line::distanceBetweenPoints(B.beg, B.end);
						col.width = Line::distanceBetweenPoints(A.beg, A.end);
					}
					else
					{
						col.length = Line::distanceBetweenPoints(A.beg, A.end);
						col.width = Line::distanceBetweenPoints(B.beg, B.end);
					}
					col.guid = CurFill.header.guid;
					GS::Array<API_Coord> coArr;
					Int32 n = BMhGetSize((GSHandle)CurMemo.coords) / Sizeof32(API_Coord);
					for (Int32 i = 1; i < n - 1; i++)
					{
						coArr.Push((*CurMemo.coords)[i]);
					}
					if (col.width < 8 && col.length < 8) {
						col.point = RoomRec::GetArrMidPoint(coArr);
						colArr.Push(col);
						RecHandler::UFill.Delete(i);
						deleted = true;
					}

				}

			}

		}
		if (!deleted)
			i++;

	}
	return colArr;
}

GS::Array<ColRec::column> ColRec::DeleteDupColumns(GS::Array<ColRec::column> colArr)
{
	for (GS::USize i = 0; i < colArr.GetSize(); i++)
	{
		for (GS::USize j = 0; j < colArr.GetSize();)
		{
			if (i != j && Line::distanceBetweenPoints(colArr.Get(i).point, colArr.Get(j).point) < 0.5)
				colArr.Delete(j);
			else j++;
		}
	}
	return colArr;
}