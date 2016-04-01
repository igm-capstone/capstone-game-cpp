#include "stdafx.h"
#include "Selector.h"
#include "Tree.h"

using namespace BehaviorTree;

Selector::Selector(Tree& tree, std::string name) :
	Composite(tree, name)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(&OnReset);
}

void Selector::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<Selector&>(bh);

	self.mCurrent = self.mChildren.begin();
}

void Selector::OnReset(Behavior& bh, void* data)
{
	auto& self = static_cast<Selector&>(bh);

	self.mStatus = BehaviorStatus::Invalid;
}

BehaviorStatus Selector::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Selector&>(bh);

	for (;;)
	{
		Behavior& child = **self.mCurrent;

		child.Tick(data);

		// stop as soon as the current child has the same status 
		// as breakstatus (either success or failure)
		if (child.GetStatus() == BehaviorStatus::Success)
		{
			self.mCurrent = self.mChildren.begin();
			return BehaviorStatus::Success;
		}

		// childrens can either complete with success or failure
		//ASSERT(child.GetStatus() == fallthrough);
		if (child.GetStatus() == BehaviorStatus::Running)
		{
			return BehaviorStatus::Running;
		}

		// otherwise, move on to the next child
		++self.mCurrent;

		// return fallthrough (success or failure) if no children left
		if (self.mCurrent == self.mChildren.end())
		{
			// self.mBehaviorTree->Stop(self, fallthrough);
			self.mCurrent = self.mChildren.begin();
			return BehaviorStatus::Failure;
		}
	}
}