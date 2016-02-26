#pragma once
#include "PathFinder\Graph.h"

class Node : public PathFinder::INode {
public:
	enum State
	{
		BLOCKED,
		CLEAN,
		GOAL,
		PATH,
		UNKNOWN,
	};


	bool hasLight;
	vec3f worldPos;

	// -10: not visited 
	//  -2: obstacle
	//  -1: walkable
	//	 0: goal
	// > 0: path to target
	State GetState() const
	{
		if (weight > 0)
		{
			return State::PATH;
		}
		
		if (weight == -1)
		{
			return State::CLEAN;
		}

		if (weight == -2)
		{
			return State::BLOCKED;
		}

		if (weight == 0)
		{
			return State::GOAL;
		}

		return State::UNKNOWN;
	}
};
