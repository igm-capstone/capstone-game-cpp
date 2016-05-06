#pragma once
#include "Composite.h"

namespace BehaviorTree
{

	class Priority : public Composite
	{

	public:
		Priority(Tree& tree, const std::string& name = "Priority");

	protected:
		Behavior* mRunning;
		std::vector<class Behavior*>::iterator mCurrent;
		std::vector<class Behavior*>::iterator mLast;

		static void OnInitialize(Behavior& self, void* data);
		static void OnReset(Behavior& bh, void* data);
		static BehaviorStatus OnUpdate(Behavior& bh, void* data);
	};

}
