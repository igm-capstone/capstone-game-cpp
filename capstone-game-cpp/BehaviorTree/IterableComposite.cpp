#include "stdafx.h"
#include "IterableComposite.h"
#include "BehaviorTree.h"

IterableComposite::IterableComposite(BehaviorTree& bt, BehaviorStatus breakStatus) : 
	mBehaviorTree(&bt),
	mBreakStatus(breakStatus)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
	SetFlushCallback(&OnFlush);
}

void IterableComposite::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<IterableComposite&>(bh);

	self.mCurrent = self.mChildren.begin();
	self.mBehaviorTree->Start(**self.mCurrent, { &OnChildComplete, &self, data });
}

void IterableComposite::OnFlush(Behavior& bh, void* data)
{
	auto& self = static_cast<IterableComposite&>(bh);

	if (self.mCurrent == self.mChildren.end())
	{
		self.mStatus = BehaviorStatus::Invalid;
	}
}

void IterableComposite::OnChildComplete(Behavior& bh, void* data, BehaviorStatus status)
{
	auto& self = static_cast<IterableComposite&>(bh);

	const Behavior& child = **self.mCurrent;

	// stop as soon as the current child has the same status 
	// as breakstatus (either success or failure)
	if (child.GetStatus() == self.mBreakStatus)
	{
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
		self.mBehaviorTree->Stop(self, fallthrough);
		return;
	}

	// start the execution of the next child 
	// and get notified on it's completion
	self.mBehaviorTree->Start(**self.mCurrent, { &OnChildComplete, &self, data });
}

BehaviorStatus IterableComposite::OnUpdate(Behavior& self, void* data)
{
	// always return running,
	// mStatus will be set to SUCCESS or FAILURE when a children is completed
	// using BehaviorTree::Stop(this, status);
	return BehaviorStatus::Running;
}