#pragma once
#include "PathFinder\Graph.h"

class Node : public PathFinder::INode {
public:
	bool hasLight;
	vec3f worldPos;
};
