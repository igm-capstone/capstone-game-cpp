#include "stdafx.h"
#include "Selector.h"
#include "BehaviorTree.h"
#include "Sequence.h"


Selector::Selector(BehaviorTree& bt, std::string name) : IterableComposite(bt, BehaviorStatus::Success, name)
{
	// set state to invalid????
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

		CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
		CHECK_EQUAL(0, selector[0].mTerminateCalled);

		selector[0].mReturnStatus = status[i];
		bt.Tick();

		CHECK_EQUAL(status[i], selector.GetStatus());
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


TEST(BehaviorTrees, Tick_SequenceParentSelectorChildren_AllSucceeds)
{
	BehaviorTree bt;
	Sequence sequence(bt);
	MockSelector selector1(bt, 2);
	MockSelector selector2(bt, 2);

	sequence.Add(selector1);
	sequence.Add(selector2);

	bt.Start(sequence);
	selector1[0].mReturnStatus = BehaviorStatus::Success;
	selector2[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Success, sequence.GetStatus());
	CHECK_EQUAL(1, selector1[0].mTerminateCalled);
	CHECK_EQUAL(0, selector1[1].mInitializeCalled);
	CHECK_EQUAL(1, selector2[0].mTerminateCalled);
	CHECK_EQUAL(0, selector2[1].mInitializeCalled);
}


TEST(BehaviorTrees, Tick_SequenceParentSelectorChildren_SecondFailsSequenceFails)
{
	BehaviorTree bt;
	Sequence sequence(bt);
	MockSelector selector1(bt, 2);
	MockSelector selector2(bt, 2);

	sequence.Add(selector1);
	sequence.Add(selector2);

	bt.Start(sequence);
	selector1[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, sequence.GetStatus());
	CHECK_EQUAL(BehaviorStatus::Success, selector1.GetStatus());
	CHECK_EQUAL(BehaviorStatus::Running, selector2.GetStatus());
	CHECK_EQUAL(1, selector1[0].mTerminateCalled);
	CHECK_EQUAL(0, selector1[1].mInitializeCalled);
	CHECK_EQUAL(1, selector2[0].mInitializeCalled);

	selector2[0].mReturnStatus = BehaviorStatus::Failure;
	selector2[1].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Failure, sequence.GetStatus());
	CHECK_EQUAL(BehaviorStatus::Success, selector1.GetStatus());
	CHECK_EQUAL(BehaviorStatus::Failure, selector2.GetStatus());
	CHECK_EQUAL(1, selector1[0].mTerminateCalled);
	CHECK_EQUAL(1, selector2[0].mTerminateCalled);
	CHECK_EQUAL(1, selector2[1].mTerminateCalled);
}
