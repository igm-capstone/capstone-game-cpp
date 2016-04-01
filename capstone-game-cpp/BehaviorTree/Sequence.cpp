#include "stdafx.h"
#include "Sequence.h"
#include "Tree.h"

using namespace BehaviorTree;

Sequence::Sequence(Tree& tree, std::string name) :
	Composite(tree, name)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(&OnReset);
}

void Sequence::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<Sequence&>(bh);

	self.mCurrent = self.mChildren.begin();
}

void Sequence::OnReset(Behavior& bh, void* data)
{
	auto& self = static_cast<Sequence&>(bh);

	self.mStatus = BehaviorStatus::Invalid;
}

BehaviorStatus Sequence::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Sequence&>(bh);

	for (;;)
	{
		Behavior& child = **self.mCurrent;

		child.Tick(data);

		// stop as soon as the current child has the same status 
		// as breakstatus (either success or failure)
		if (child.GetStatus() == BehaviorStatus::Failure)
		{
			self.mCurrent = self.mChildren.begin();
			return BehaviorStatus::Failure;
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
			self.mCurrent = self.mChildren.begin();
			return BehaviorStatus::Success;
		}
	}
}