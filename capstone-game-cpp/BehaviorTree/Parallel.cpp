#include "stdafx.h"
#include "Parallel.h"
#include "Tree.h"

using namespace BehaviorTree;

Parallel::Parallel(Tree& tree, std::string name) :
	Composite(tree, name)
{
	//SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(&OnReset);
}

void Parallel::OnInitialize(Behavior& bh, void* data)
{
}

void Parallel::OnReset(Behavior& bh, void* data)
{
	auto& self = static_cast<Parallel&>(bh);

	self.mStatus = BehaviorStatus::Invalid;
}

BehaviorStatus Parallel::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Parallel&>(bh);

	// start assuming all children are gonna succed and update
	// result whenever a child succedes or ends the frame 
	// running
	BehaviorStatus result = BehaviorStatus::Success;

	for (auto child : self.mChildren)
	{
		BehaviorStatus childStatus = child->Tick(data);

		// stop as soon as the current child has the same status 
		// as breakstatus (either success or failure)
		if (childStatus == BehaviorStatus::Failure)
		{
			result = BehaviorStatus::Failure;
			continue;
		}

		// childrens can either complete with success or failure
		//ASSERT(child.GetStatus() == fallthrough);
		if (childStatus == BehaviorStatus::Running)
		{
			if (result == BehaviorStatus::Success)
			{
				result = BehaviorStatus::Running;
			}

			continue;
		}
	}

	return result;
}