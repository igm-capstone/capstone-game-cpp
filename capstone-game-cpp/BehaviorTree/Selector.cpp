#include "stdafx.h"
#include "Selector.h"
#include "BehaviorTree.h"
#include "Sequence.h"


Selector::Selector(BehaviorTree& bt) : mBehaviorTree(&bt)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
}

void Selector::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<Selector&>(bh);

	self.mCurrent = self.mChildren.begin();
	self.mBehaviorTree->Start(**self.mCurrent, &OnChildComplete, &self);
}

void Selector::OnChildComplete(void* observerData, BehaviorStatus status)
{
	auto& self = *static_cast<Selector*>(observerData);

	const Behavior& child = **self.mCurrent;

	// stop as soon as the current child has succeeded
	if (child.GetStatus() == BehaviorStatus::Success)
	{
		self.mBehaviorTree->Stop(self, BehaviorStatus::Success);
		return;
	}

	// childrens can either complete with success or failure
	ASSERT(child.GetStatus() == BehaviorStatus::Failure);

	// otherwise, move on to the next child
	++self.mCurrent;

	// return failure if no children left
	if (self.mCurrent == self.mChildren.end())
	{
		self.mBehaviorTree->Stop(self, BehaviorStatus::Failure);
		return;
	}
	
	// start the execution of the next child 
	// and get notified on it's completion
	self.mBehaviorTree->Start(**self.mCurrent, &OnChildComplete, &self);
}

BehaviorStatus Selector::OnUpdate(Behavior& bh, void* data)
{
	// always return running,
	// mStatus will be set to SUCCESS or FAILURE when a children is completed
	// using BehaviorTree::Stop(this, status);
	return BehaviorStatus::Running;
}


typedef MockComposite<Selector> MockSelector;

TEST(BehaviorTrees, Tick_PassThroughToSibling_ReturnTerminated)
{
	BehaviorStatus status[] = { BehaviorStatus::Success, BehaviorStatus::Failure };
	for (size_t i = 0; i < 2; i++)
	{
		BehaviorTree bt;
		MockSelector selector(bt, 1);

		bt.Start(selector);
		bt.Tick();

		CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Running);
		CHECK_EQUAL(0, selector[0].mTerminateCalled);

		selector[0].mReturnStatus = status[i];
		bt.Tick();

		CHECK_EQUAL(selector.GetStatus(), status[i]);
		CHECK_EQUAL(1, selector[0].mTerminateCalled);
	}
}

TEST(BehaviorTrees, Tick_SelectorFirstFailsSecondRuns_EndsWithSuccess)
{
	BehaviorTree bt;
	MockSelector selector(bt, 2);

	bt.Start(selector);
	bt.Tick();

	CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(0, selector[0].mTerminateCalled);
	CHECK_EQUAL(0, selector[1].mInitializeCalled);

	selector[0].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();

	CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(1, selector[0].mTerminateCalled);
	CHECK_EQUAL(1, selector[1].mInitializeCalled);


	selector[1].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Success);
	CHECK_EQUAL(1, selector[0].mTerminateCalled);
	CHECK_EQUAL(1, selector[1].mTerminateCalled);
}

TEST(BehaviorTrees, Tick_SelectorSecondNotInitialized_ReturnSuccess)
{
	BehaviorTree bt;
	MockSelector selector(bt, 2);

	bt.Start(selector);
	bt.Tick();

	CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(0, selector[0].mTerminateCalled);

	selector[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(selector.GetStatus(), BehaviorStatus::Success);
	CHECK_EQUAL(1, selector[0].mTerminateCalled);
	CHECK_EQUAL(0, selector[1].mInitializeCalled);
}