#include "stdafx.h"
#include "PrioritySelector.h"
#include "BehaviorTree/BehaviorTree.h"

PrioritySelector::PrioritySelector(BehaviorTree& bt, std::string name) : IterableComposite(bt, BehaviorStatus::Success, name)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetResetCallback(nullptr);
}

void PrioritySelector::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<PrioritySelector&>(bh);

	//self.mCurrent = self.mChildren.begin();
	//self.mLastChildComplete = self.mCurrent;
	self.mLastChildComplete = self.mChildren.begin();

	//self.mBehaviorTree->Start(**self.mCurrent, { &OnChildComplete, &self, data });
}

void PrioritySelector::OnChildComplete(Behavior& bh, void* data, BehaviorStatus status)
{
	auto& self = static_cast<PrioritySelector&>(bh);

	const Behavior& child = **self.mCurrent;

	// stop as soon as the current child has the same status 
	// as breakstatus (either success or failure)
	if (child.GetStatus() == self.mBreakStatus)
	{
		self.mLastChildComplete = self.mCurrent;
		self.mBehaviorTree->Stop(self, self.mBreakStatus);
		return;
	}

	// fallthrough is the oposite status to breakstatus
	auto fallthrough = self.mBreakStatus == BehaviorStatus::Success ?
		BehaviorStatus::Failure : BehaviorStatus::Success;

	// childrens can either complete with success or failure
	ASSERT(child.GetStatus() == fallthrough);

	// otherwise, move on to the next child
	++self.mCurrent;

	// return fallthrough (success or failure) if no children left
	if (self.mCurrent == self.mChildren.end())
	{
		self.mLastChildComplete = self.mCurrent;
		self.mBehaviorTree->Stop(self, fallthrough);
		return;
	}

	// start the execution of the next child 
	// and get notified on it's completion
	self.mBehaviorTree->Start(**self.mCurrent, { &OnChildComplete, &self, data });
}


BehaviorStatus PrioritySelector::OnUpdate(Behavior& bh, void* data)
{
	auto& self = static_cast<PrioritySelector&>(bh);

	//for (auto child = self.mChildren.begin(); child < self.mLastChildComplete; ++child)
	//{
	//	(*child)->Reset(data);
	//}

	//self.mCurrent = self.mChildren.begin();
	//self.mLastChildComplete = self.mCurrent;

	// always return running,
	// mStatus will be set to SUCCESS or FAILURE when a children is completed
	// using BehaviorTree::Stop(this, status);
	//return BehaviorStatus::Running;

	self.mCurrent = self.mChildren.begin();
	
	for (;;)
	{
		Behavior& child = **self.mCurrent;

		child.Tick(data);

		// stop as soon as the current child has the same status 
		// as breakstatus (either success or failure)
		if (child.GetStatus() != BehaviorStatus::Failure)
		{
			auto it = self.mCurrent;
			for (auto c = ++it; c < self.mLastChildComplete; ++c)
			{
				(*c)->Reset(data);
			}	

			self.mLastChildComplete = self.mCurrent;
			return child.GetStatus();
		}
		
		// otherwise, move on to the next child
		++self.mCurrent;

		// return fallthrough (success or failure) if no children left
		if (self.mCurrent == self.mChildren.end())
		{
			return BehaviorStatus::Failure;
		}
	}
}