#include "stdafx.h"
#include "Priority.h"
#include "Tree.h"

using namespace BehaviorTree;

Priority::Priority(Tree& tree, const std::string& name)
	: Composite(tree, name)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(&OnReset);
}

void Priority::OnInitialize(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Priority&>(bh);
	self.mCurrent = self.mChildren.begin();
	self.mLast = self.mChildren.begin();
}


void Priority::OnReset(Behavior& bh, void* data)
{
	auto& self = static_cast<Priority&>(bh);

	self.mStatus = BehaviorStatus::Invalid;
}

BehaviorStatus Priority::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Priority&>(bh);

	self.mCurrent = self.mChildren.begin();

	for (;;)
	{
		Behavior& child = **self.mCurrent;

		child.Tick(data);

		if (child.GetStatus() != BehaviorStatus::Failure)
		{
			auto it = self.mCurrent;
			for (++it; it <= self.mLast; ++it)
			{
				(*it)->Reset(data);
			}

			self.mLast = self.mCurrent;
			return child.GetStatus();
		}

		// otherwise, move on to the next child
		++self.mCurrent;

		// return fallthrough (success or failure) if no children left
		if (self.mCurrent == self.mChildren.end())
		{
			self.mCurrent = self.mChildren.begin();
			return BehaviorStatus::Failure;
		}
	}
}
