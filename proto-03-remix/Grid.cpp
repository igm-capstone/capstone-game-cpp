#include <windows.h>
#include "Grid.h"
#include <cmath>

Grid::Grid()
	: pathFinder(graph)
{
	//Create nodes
	for (int x = 0; x < numSpheresX; x++) {
		for (int y = 0; y < numSpheresY; y++) {
			Vector3 nodePos = startingCorner + Vector3((nodeRadius * 2) * x + nodeRadius, (nodeRadius * 2) * y + nodeRadius, 0);
			Vector2 coord = Vector2((float)x, (float)y);
			graph.grid[x][y].coord = coord;
			graph.grid[x][y].position = nodePos;
		}
	}

	UpdateGrid();
}

Node* Grid::GetNodeAt(Vector3 pos)
{
	float x = (pos.x - nodeRadius - startingCorner.x) / (nodeRadius * 2);
	float y = (pos.y - nodeRadius - startingCorner.y) / (nodeRadius * 2);

	return &(graph.grid[(int)round(x)][(int)round(y)]);
}

void Grid::UpdateGrid() {

}

SearchResult<Node> Grid::GetPath(Vector3 from, Vector3 to) {

	// we want the path backward because we want the
	// pathfinder to retain cache for multiple queries 
	// towards the same target (robots chasing player).
	auto end = GetNodeAt(from);
	auto start = GetNodeAt(to);

	auto result = pathFinder.FindPath(start, end);

	return result;
}

Grid::~Grid()
{
}
