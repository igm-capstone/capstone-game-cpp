#pragma once
#include "Behavior.h"

namespace BehaviorTree
{

	class Decorator : public Behavior
	{
	public:
		Decorator(Tree& tree, Behavior& child, std::string name = "Decorator");

	protected:
		Behavior& mChild;
	};

}
