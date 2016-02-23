#include "stdafx.h"
#include "Condition.h"
#include "TestSuite.h"
#include "BehaviorTree.h"


BehaviorStatus Condition::Update()
{
	if (mCanRun != nullptr && mCanRun())
	{
		return BehaviorStatus::Success;
	}
	
	return BehaviorStatus::Failure;
}


TEST(BehaviorTrees, Tick_ConditionTrue_ReturnsSuccess)
{
	int health = 10;
	Condition condition;
	BehaviorTree bt;

	condition.mCanRun = [health] () {
		return health < 50;
	};

	bt.Start(condition);
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Success, condition.mStatus);
}


TEST(BehaviorTrees, Tick_ConditionFalse_ReturnsFailure)
{
	int health = 60;
	Condition condition;
	BehaviorTree bt;

	condition.mCanRun = [health]() {
		return health < 50;
	};

	bt.Start(condition);
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Failure, condition.mStatus);
}