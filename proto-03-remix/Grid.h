#pragma once
#include "PathFinder\Fringe.h"
#include "GraphicsMath\Vector.hpp"
#include "Node.h"
using namespace cliqCity::graphicsMath;

const Vector3 startingCorner = Vector3(-52, -34, 0);
const int maxSizeX = 105;
const int maxSizeY = 68;
const float nodeRadius = 1.0f;

const int numSpheresX = 52;//round(maxSizeX / nodeRadius) / 2;
const int numSpheresY = 34;// round(maxSizeY / nodeRadius) / 2;

#pragma once
class Grid
{
public:
	PathFinder::Fringe<Node, numSpheresX, numSpheresY> pathFinder;
	
public:
	static Grid& getInstance()
	{
		static Grid instance;
		return instance;
	}

	void UpdateGrid();
	Node& getNodeAtPos(Vector3 pos);
	void GetFringePath(Vector3 from, Vector3 to);

private:
	Grid();
	Grid(Grid const&) = delete;
	void operator=(Grid const&) = delete;
	~Grid();


};

