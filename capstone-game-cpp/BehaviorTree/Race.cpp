#include "stdafx.h"
#include "Race.h"
#include "Tree.h"

using namespace BehaviorTree;

Race::Race(Tree& tree, std::string name) :
	Composite(tree, name)
{
	//SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(&OnReset);
}

void Race::OnInitialize(Behavior& bh, void* data)
{
}

void Race::OnReset(Behavior& bh, void* data)
{
	auto& self = static_cast<Race&>(bh);

	self.mStatus = BehaviorStatus::Invalid;
}

BehaviorStatus Race::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Race&>(bh);

	// start assuming all children are gonna fail and update
	// result whenever a child succedes or ends the frame 
	// running
	BehaviorStatus result = BehaviorStatus::Failure;

	for (auto child : self.mChildren)
	{
		BehaviorStatus childStatus = child->Tick(data);

		// stop as soon as the current child has the same status 
		// as breakstatus (either success or failure)
		if (childStatus == BehaviorStatus::Success)
		{
			result = BehaviorStatus::Success;
			continue;
		}

		// childrens can either complete with success or failure
		//ASSERT(child.GetStatus() == fallthrough);
		if (childStatus == BehaviorStatus::Running)
		{
			if (result == BehaviorStatus::Failure)
			{
				result = BehaviorStatus::Running;
			}

			continue;
		}
	}

	return result;
}