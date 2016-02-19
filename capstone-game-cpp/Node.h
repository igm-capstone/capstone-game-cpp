#pragma once
#include "PathFinder\Graph.h"

class Node : public PathFinder::INode {
public:
	vec3f worldPos;
	bool hasLight;

};
