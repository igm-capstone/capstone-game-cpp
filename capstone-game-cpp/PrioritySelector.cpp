#include "stdafx.h"
#include "PrioritySelector.h"

PrioritySelector::PrioritySelector(BehaviorTree& bt, std::string name) : IterableComposite(bt, BehaviorStatus::Success, name)
{
	SetUpdateCallback(&OnUpdate);
}


BehaviorStatus PrioritySelector::OnUpdate(Behavior& bh, void* data)
{
	auto& self = static_cast<PrioritySelector&>(bh);

	for (auto child : self.mChildren)
	{
		child->Reset(data);
	}

	self.mCurrent = self.mChildren.begin();

	// always return running,
	// mStatus will be set to SUCCESS or FAILURE when a children is completed
	// using BehaviorTree::Stop(this, status);
	return BehaviorStatus::Running;
}