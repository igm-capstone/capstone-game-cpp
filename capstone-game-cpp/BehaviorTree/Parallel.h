#pragma once
#include "Composite.h"

namespace BehaviorTree
{

	class Parallel : public Composite
	{
	public:
		Parallel(Tree& tree, std::string name = "Parallel");

	protected:

		static void OnInitialize(Behavior& self, void* data);
		static void OnReset(Behavior& bh, void* data);
		static BehaviorStatus OnUpdate(Behavior& self, void* data);
	};

}