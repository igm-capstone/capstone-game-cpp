#pragma once
#include "Decorator.h"

namespace BehaviorTree
{

	class Mute : public Decorator
	{
	public:
		Mute(Tree& tree, Behavior* child, std::string name = "Mute");

	protected:
		static BehaviorStatus OnUpdate(Behavior& bh, void* data);
	};

}
