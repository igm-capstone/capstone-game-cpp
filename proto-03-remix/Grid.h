#include "GraphicsMath\Vector.hpp"
using namespace cliqCity::graphicsMath;

#pragma once
class Grid
{
	struct Node {
		Vector3 position;
		Vector2 coord;
		float weight;

		Node(Vector3 position, Vector2 coord, float weight) {
			this->position = position;
			this->coord = coord;
			this->weight = weight;
		}
	};

public:
	const float nodeRadius = 0.5f;

private:
	Node ***areaOfNodes;
	int numSpheresX, numSpheresY;
	Vector3 startingCorner;

public:
	static Grid& getInstance()
	{
		static Grid instance;
		return instance;
	}

	void UpdateGrid();
	Node& getNodeAtPos(Vector3 pos);

private:
	Grid();
	Grid(Grid const&) = delete;
	void operator=(Grid const&) = delete;
	~Grid();


};

