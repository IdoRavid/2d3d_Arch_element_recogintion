#include "Polygons.h"
#include <vector>
using namespace std;

bool Polygons::isCoordInSimplePoly(API_Coord coord, std::vector<API_Coord> poly) {
	size_t polyCount = poly.size();
	size_t j = polyCount - 1;//Used for comprison with previous coord in the array.
	bool result = false;
	for (Int32 i = 0; i < polyCount; i++) {
		if (poly[i].x != 0 && poly[i].y != 0) {
			if ((poly[i].y < coord.y && poly[j].y >= coord.y) || (poly[j].y < coord.y && poly[i].y >= coord.y)) {
				if (poly[i].x + (coord.y - poly[i].y) / (poly[j].y - poly[i].y) * (poly[j].x - poly[i].x) < coord.x) {
					result = !result;
				}
			}
			j = i;
		}
	}
	return result;
}

bool Polygons::isCoordInPolygon(API_Coord coord, API_Coord** coords) {
	bool bigPolyChecked = false; //Indicating whether we are after checking the first, big poly.
	Int32 n = BMhGetSize((GSHandle)coords) / Sizeof32(API_Coord);//gets the number of items at coords array into n.
	std::vector<API_Coord> currentPoly(0);
	for (Int32 i = 0; i < n; i++) {
		bool currentIsFlag = false;//Indicating whether current coord is a flag coord
		for (Int32 d = 0; d < i; d++) {
			if ((*coords)[d].x == (*coords)[i].x && (*coords)[d].y == (*coords)[i].y) {
				bool currentPolyResult = Polygons::isCoordInSimplePoly(coord, currentPoly);
				currentIsFlag = true;
				currentPoly.clear();//We are moving to a new poly
				if (bigPolyChecked && currentPolyResult == true)
					return false;//If this is an inner poly and the coord is in it, it's outside of the big poly
				if (!bigPolyChecked && currentPolyResult == false)
					return false;
				else if (!bigPolyChecked)
					bigPolyChecked = true;
			}
		}
		API_Coord c;
		c.x = (*coords)[i].x;
		c.y = (*coords)[i].y;
		if (currentIsFlag == false)
			currentPoly.push_back(c);
	}
	return true;
}

API_Coord Polygons::centroidCoord(API_Coord** coords)//Gets coord array and returns the middle coord.
{
	double x = 0, y = 0, dontTakeInACount = 0;
	Int32 n = BMhGetSize((GSHandle)coords) / Sizeof32(API_Coord);//gets the number of items at coords array into n.
	for (Int32 a = 0; a < n; a++) {//From some reason first 2 coords are always 0,0.
		if ((*coords)[a].x == 0 && (*coords)[a].y == 0) {
			dontTakeInACount++;//We don't want to take 0,0 in account when calculating sum coord.
			continue;
		}
		bool alreadyAdded = false;//Indicating whether current coord has already been added to our calculation. If not, let's add it.
		for (Int32 d = 0; d < a; d++) {
			if ((*coords)[d].x == (*coords)[a].x && (*coords)[d].y == (*coords)[a].y) {
				alreadyAdded = true;
				dontTakeInACount++;
				break;
			}
		}
		if (!alreadyAdded) {
			x += (*coords)[a].x;
			y += (*coords)[a].y;
		}
	}
	x /= (n - dontTakeInACount);//first cord is always 0 and last cord is same as second cord.
	y /= (n - dontTakeInACount);
	API_Coord cord;
	cord.x = x;
	cord.y = y;
	return cord;
}

API_Coord Polygons::centroidCoord(std::vector<API_Coord> coords)//Gets coord list and returns the middle coord.
{
	double x = 0, y = 0;
	size_t n = coords.size();//gets the number of items at coords array into n.
	for (Int32 a = 0; a < n; a++) {//From some reason first 2 coords are always 0,0.
		x += (coords)[a].x;
		y += (coords)[a].y;
	}
	x /= n;//first cord is always 0 and last cord is same as second cord.
	y /= n;
	API_Coord cord;
	cord.x = x;
	cord.y = y;
	return cord;
}

API_Coord Polygons::innerCentroid(API_Coord** coords,short floorInd) {//Gets coord list and returns the inner centroid coord.
	API_Coord naturalCentroid = Polygons::centroidCoord(coords);
	std::vector<API_Coord> centroidsOfTriangles(0);
	Int32 n = BMhGetSize((GSHandle)coords) / Sizeof32(API_Coord);//gets the number of items at coords array into n.
	if (isCoordInPolygon(naturalCentroid, coords) && IsCoordOnWall(naturalCentroid,floorInd)==false)
		return naturalCentroid;
	else {//If the natural centroid is outer centroid, we need to find the other best centroid:
		for (Int32 a = 0; a < n; a++) {
			std::vector<API_Coord> triangle(0);//A triangle coords array to find a centroid to.
			triangle.push_back((*coords)[a]);
			if (a + 2 < n) {//Keep building the triangle
				triangle.push_back((*coords)[a + 1]);
				triangle.push_back((*coords)[a + 2]);
			}
			if (a + 2 == n || a + 1 == n)
				triangle.push_back((*coords)[0]);//If we are at the end of the coordArr, let's use the beginning of the arr
			if (a + 1 == n)
				triangle.push_back((*coords)[1]);
			API_Coord currentTriangleCentroid = centroidCoord(triangle);
			if (isCoordInPolygon(currentTriangleCentroid, coords))
				centroidsOfTriangles.push_back(currentTriangleCentroid);
			triangle.clear();//Clearing the triangle to create a new one.
		}
		double smallestDistance = 10000;//The smallest distance between a triangle centroid which is in the polygon to the polygon's natiral centroid
		Int32 smallestDistanceTrinagleInd = -1;//The ind of the centroid of the triangle with smallest distance to natural cetnroid of the polygon
		for (Int32 e = 0; e < centroidsOfTriangles.size(); e++) {
			double dist = sqrt(pow(naturalCentroid.x - centroidsOfTriangles[e].x, 2) + pow(naturalCentroid.y - centroidsOfTriangles[e].y, 2));
			if (smallestDistance > dist && (floorInd != 100 && IsCoordOnWall(centroidsOfTriangles[e],floorInd) == false)) {//If the current coord distance to the natural polygon centroid is smallest of the current smallest distance we know to the natural centroid:
				smallestDistance = dist;
				smallestDistanceTrinagleInd = e;//This is the index of the centroid with smallest dest to natural cenroid we know so far
			}
		}
		return centroidsOfTriangles[smallestDistanceTrinagleInd];
	}
}

bool Polygons::IsCoordOnWall(API_Coord coord, short floorInd) {//Gets a coord and returns true if it is on a wall at the given floor ind
	GS::Array<API_Guid> elemList;//Array of all elements is scene
	ACAPI_Element_GetElemList(API_WallID, &elemList, APIFilt_OnVisLayer);//Getting all visible floor elements into elemList
	for (GS::USize i = 0; i < elemList.GetSize(); i++) {
		API_Element currentWall;
		API_ElementMemo m;
		BNZeroMemory(&currentWall, sizeof(API_Element)); //Clearing memory for it
		BNZeroMemory(&m, sizeof(API_ElementMemo)); //Clearing memory for it
		currentWall.header.guid = elemList[i];
		ACAPI_Element_Get(&currentWall);
		if (currentWall.header.floorInd == floorInd) {//If this wall is on the same floor we are checking the coord for.
			ACAPI_Element_GetMemo(elemList[i], &m);
			bool isInPoly = isCoordInPolygon(coord, m.coords);
			if (isInPoly)
				return true;
		}
	}
	return false;
}

API_Coord** Polygons::cArrTocoords(GS::Array<API_Coord> cArr) {

	API_Coord** emptyCoords, ** coords;
	emptyCoords = nullptr;
	if (cArr.GetSize() == 0)
		return emptyCoords;


	coords = (API_Coord**)BMAllocateHandle((cArr.GetSize() + 2) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
	(*coords)[0].x = 0.0;
	(*coords)[0].y = 0.0;

	for (GS::USize i = 0; i < cArr.GetSize(); i++)
	{
		(*coords)[int(i + 1)] = cArr.Get(i);
	}
	(*coords)[int(cArr.GetSize() + 1)] = cArr.Get(0);
	return coords;
}