#pragma once
#include "Composite.h"

namespace BehaviorTree
{

	class Race : public Composite
	{
	public:
		Race(Tree& tree, std::string name = "Race");

	protected:

		static void OnInitialize(Behavior& self, void* data);
		static void OnReset(Behavior& bh, void* data);
		static BehaviorStatus OnUpdate(Behavior& self, void* data);
	};

}