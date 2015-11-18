#pragma once
#include "PathFinder\Graph.h"
#include "GraphicsMath\Vector.hpp"
using namespace cliqCity::graphicsMath;

class Node : public PathFinder::INode {
public:
	Vector2 coord;
	Vector3 position;
	bool hasLight;

public:
	Node() {};
};
