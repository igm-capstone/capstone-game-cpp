#include "Grid.h"
#include <cmath>

Grid::Grid()
{
	//Hardcoded for now?
	startingCorner = Vector3(-52, -34, 0);
	Vector2 maxSize = Vector2(105, 68);

	numSpheresX = round(maxSize.x / nodeRadius) / 2;
	numSpheresY = round(maxSize.y / nodeRadius) / 2;

	//Alloc
	areaOfNodes = (Node***)malloc(numSpheresX*sizeof(Node**));
	for (int i = 0; i < numSpheresX; i++)
		areaOfNodes[i] = (Node**)malloc(numSpheresY*sizeof(Node*));
	
	//Create nodes
	for (int x = 0; x < numSpheresX; x++) {
		for (int y = 0; y < numSpheresY; y++) {
			Vector3 nodePos = startingCorner + Vector3((nodeRadius * 2) * x + nodeRadius, (nodeRadius * 2) * y + nodeRadius, 0);
			areaOfNodes[x][y] = new Node(nodePos, Vector2(x, y), 1);
		}
	}

	UpdateGrid();
}

Grid::Node& Grid::getNodeAtPos(Vector3 pos)
{
	float x = (pos.x - nodeRadius - startingCorner.x) / (nodeRadius * 2);
	float y = (pos.y - nodeRadius - startingCorner.y) / (nodeRadius * 2);

	return *areaOfNodes[(int)round(x)][(int)round(y)];
}

void Grid::UpdateGrid() {

}

Grid::~Grid()
{
	//Delete
	for (int x = 0; x < numSpheresX; x++) {
		for (int y = 0; y < numSpheresY; y++) {
			delete(areaOfNodes[x][y]);
		}
	}

	//Dealloc
	for (int i = 0; i < numSpheresX; i++)
	{
		free(areaOfNodes[i]);
	}

	free(areaOfNodes);
}
