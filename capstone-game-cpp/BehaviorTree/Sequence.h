#pragma once
#include "Composite.h"

namespace BehaviorTree
{

	class Sequence : public Composite
	{
	public:
		Sequence(Tree& tree, std::string name = "Sequence");

	protected:
		/*class BehaviorTree* mBehaviorTree;*/
		std::vector<class Behavior*>::iterator mCurrent;

		static void OnInitialize(Behavior& self, void* data);
		static void OnReset(Behavior& bh, void* data);
		static BehaviorStatus OnUpdate(Behavior& self, void* data);
	};

}