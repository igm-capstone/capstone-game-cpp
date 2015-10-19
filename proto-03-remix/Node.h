#pragma once
#include "PathFinder\Graph.h"
#include "GraphicsMath\Vector.hpp"
using namespace cliqCity::graphicsMath;

class Node : public PathFinder::INode {
public:
	Vector3 position;
	Vector2 coord;

public:
	Node() {};
};
