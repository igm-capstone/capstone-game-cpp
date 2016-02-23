#include "stdafx.h"
#include "Behavior.h"
#include "BehaviorTree.h"

Behavior::Behavior(): mStatus(BehaviorStatus::Invalid)
{
}


Behavior::~Behavior()
{
}

BehaviorStatus Behavior::Tick()
{
	if (mStatus == BehaviorStatus::Invalid)
	{
		OnInitialize();
	}

	mStatus = Update();

	if (mStatus != BehaviorStatus::Running)
	{
		OnTerminate(mStatus);
	}

	return mStatus;
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
