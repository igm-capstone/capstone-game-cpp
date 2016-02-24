#include "stdafx.h"
#include "Condition.h"
#include "TestSuite.h"
#include "BehaviorTree.h"


Condition::Condition()
{
	SetUpdateCallback(&Update);
}


BehaviorStatus Condition::Update(Behavior& bh, void* data)
{
	auto& self = static_cast<Condition&>(bh);

	if (self.mOnCondition && self.mOnCondition(bh, data))
	{
		return BehaviorStatus::Success;
	}
	
	return BehaviorStatus::Failure;
}


bool lessThan50(Behavior &self, void* data)
{
	int value = *static_cast<int*>(data);
	return value < 50;
}

TEST(BehaviorTrees, Tick_ConditionTrue_ReturnsSuccess)
{
	int health = 10;
	Condition condition;
	BehaviorTree bt;

	condition.SetConditionCallback(&lessThan50);

	bt.Start(condition);
	bt.Tick(&health);

	CHECK_EQUAL(BehaviorStatus::Success, condition.GetStatus());
}


TEST(BehaviorTrees, Tick_ConditionFalse_ReturnsFailure)
{
	int health = 60;
	Condition condition;
	BehaviorTree bt;

	condition.SetConditionCallback(&lessThan50);

	bt.Start(condition);
	bt.Tick(&health);

	CHECK_EQUAL(BehaviorStatus::Failure, condition.GetStatus());
}
