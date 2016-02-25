#include "stdafx.h"
#include "Behavior.h"
#include "BehaviorTree.h"

Behavior::Behavior(std::string name):
	mName(name),
	mStatus(BehaviorStatus::Invalid), 
	mObserver(BehaviorObserver::Default()),
	mOnUpdate(nullptr), 
	mOnReset(nullptr), 
	mOnInitialize(nullptr), 
	mOnTerminate(nullptr)
{
}

Behavior::~Behavior()
{
}

BehaviorStatus Behavior::Tick(void* userData)
{


	if (mOnInitialize && mStatus == BehaviorStatus::Invalid)
	{
		mOnInitialize(*this, userData);
	}

	if (mOnUpdate)
	{
		mStatus = mOnUpdate(*this, userData);
	}

	if (mOnTerminate && mStatus != BehaviorStatus::Running)
	{
		mOnTerminate(*this, userData, mStatus);
	}

	return mStatus;
}

void Behavior::Reset(void* data)
{
	if (mOnReset && mStatus != BehaviorStatus::Invalid)
	{
		mOnReset(*this, data);
	}
}

TEST(BehaviorTrees, TaskInitialize)
{
	MockBehavior t;
	BehaviorTree bt;

	bt.Start(t);
	CHECK_EQUAL(0, t.mInitializeCalled);

	bt.Tick();
	CHECK_EQUAL(1, t.mInitializeCalled);
};

TEST(BehaviorTrees, TaskUpdate)
{
	MockBehavior t;
	BehaviorTree bt;

	bt.Start(t);
	bt.Tick();
	CHECK_EQUAL(1, t.mUpdateCalled);

	t.mReturnStatus = BehaviorStatus::Success;
	bt.Tick();
	CHECK_EQUAL(2, t.mUpdateCalled);
};

TEST(BehaviorTrees, TaskTerminate)
{
	MockBehavior t;
	BehaviorTree bt;

	bt.Start(t);
	bt.Tick();
	CHECK_EQUAL(0, t.mTerminateCalled);

	t.mReturnStatus = BehaviorStatus::Success;
	bt.Tick();
	CHECK_EQUAL(1, t.mTerminateCalled);
};
