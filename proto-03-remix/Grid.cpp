#include <windows.h>
#include "Grid.h"
#include <cmath>

Grid::Grid()
{
	//Create nodes
	for (int x = 0; x < numSpheresX; x++) {
		for (int y = 0; y < numSpheresY; y++) {
			Vector3 nodePos = startingCorner + Vector3((nodeRadius * 2) * x + nodeRadius, (nodeRadius * 2) * y + nodeRadius, 0);
			Vector2 coord = Vector2((float)x, (float)y);
			pathFinder.graph.grid[x][y].coord = coord;
			pathFinder.graph.grid[x][y].position = nodePos;
		}
	}

	UpdateGrid();
}

Node& Grid::GetNodeAt(Vector3 pos)
{
	float x = (pos.x - nodeRadius - startingCorner.x) / (nodeRadius * 2);
	float y = (pos.y - nodeRadius - startingCorner.y) / (nodeRadius * 2);

	return pathFinder.graph.grid[(int)round(x)][(int)round(y)];
}

void Grid::UpdateGrid() {

}

SearchResult<Node> Grid::GetPath(Vector3 from, Vector3 to) {

	// we want the path backward because we want the
	// pathfinder to retain cache for multiple queries 
	// towards the same target (robots chasing player).
	auto end = GetNodeAt(from);
	auto start = GetNodeAt(to);

	auto result = pathFinder.FindPath(&start, &end);

	return result;

	std::stringstream ss;
	for (int y = 0; y < numSpheresY; y++)
	{
		for (int x = 0; x < numSpheresX; x++)
		{
			bool inPath = false;
			for (auto it = result.path.begin(); it != result.path.end(); ++it)
			{
				if (**it == pathFinder.graph.grid[x][y])
				{
					inPath = true;
					break;
				}
			}

			ss << " " << (inPath ? 'X' : pathFinder.graph.grid[x][y].weight > 1 ? '#' : 'O');
		}
		ss << std::endl;
	}

	TRACE(ss.str());
}

Grid::~Grid()
{
}
