#include "stdafx.h"
#include "Action.h"
#include "TestSuite.h"
#include "BehaviorTree.h"

//
//BehaviorStatus Action::Update()
//{
//	if (mUpdate)
//	{
//		mStatus = mUpdate();
//	}
//	else
//	{
//		mStatus = BehaviorStatus::Failure;
//	}
//
//	return mStatus;
//}
//
//
//TEST(BehaviorTrees, Tick_HasNoUpdate_ReturnFailure)
//{
//	BehaviorTree bt;
//	Action action;
//
//	bt.Start(action);
//	bt.Tick();
//	
//	CHECK_EQUAL(action.mStatus, BehaviorStatus::Failure);
//}
//
//
//TEST(BehaviorTrees, Tick_HasUpdate_ReturnUpdateStatus)
//{
//	BehaviorTree bt;
//	Action action;
//	bt.Start(action);
//
//	action.mUpdate = []()
//	{
//		return BehaviorStatus::Success;
//	};
//
//	bt.Tick();
//	CHECK_EQUAL(action.mStatus, BehaviorStatus::Success);
//
//	action.mUpdate = []()
//	{
//		return BehaviorStatus::Running;
//	};
//
//	bt.Tick();
//	CHECK_EQUAL(action.mStatus, BehaviorStatus::Running);
//}