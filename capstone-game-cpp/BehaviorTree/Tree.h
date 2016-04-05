#pragma once
#include "Behavior.h"
#include <deque>

namespace BehaviorTree
{

	class Tree : public Behavior
	{
	public:
		Tree(std::string name = "Root");

		void Start(Behavior& bh);
	protected:
		std::deque<Behavior*> mBehaviors;
		Behavior* mRootBehavior;

		static BehaviorStatus OnUpdate(Behavior& bh, void* userData);
	};

}