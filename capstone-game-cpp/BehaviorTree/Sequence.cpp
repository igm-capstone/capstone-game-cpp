#include "stdafx.h"
#include "Sequence.h"
#include "BehaviorTree.h"
#include "Selector.h"

Sequence::Sequence(BehaviorTree& bt, std::string name) : IterableComposite(bt, BehaviorStatus::Failure, name)
{
}

typedef MockComposite<Sequence> MockSequence;

TEST(BehaviorTrees, SequenceOnePassThrough)
{
	BehaviorStatus status[2] = { BehaviorStatus::Success, BehaviorStatus::Failure };
	for (int i = 0; i<2; ++i)
	{
		BehaviorTree bt;
		MockSequence seq(bt, 1);

		bt.Start(seq);
		bt.Tick();
		CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Running);
		CHECK_EQUAL(0, seq[0].mTerminateCalled);

		seq[0].mReturnStatus = status[i];
		bt.Tick();
		CHECK_EQUAL(seq.GetStatus(), status[i]);
		CHECK_EQUAL(1, seq[0].mTerminateCalled);
	}
}

TEST(BehaviorTrees, Tick_SequenceTwoFails_ReturnsFailure)
{
	BehaviorTree bt;
	MockSequence seq(bt, 2);

	bt.Start(seq);
	bt.Tick();

	CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(0, seq[0].mTerminateCalled);

	seq[0].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();

	CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Failure);
	CHECK_EQUAL(1, seq[0].mTerminateCalled);
	CHECK_EQUAL(0, seq[1].mTerminateCalled);
}

TEST(BehaviorTrees, Tick_TwoChildrenSucceed_ReturnSuccess)
{
	BehaviorTree bt;
	MockSequence seq(bt, 2);

	bt.Start(seq);
	bt.Tick();
	CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(0, seq[0].mTerminateCalled);

	seq[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Running);
	CHECK_EQUAL(1, seq[0].mTerminateCalled);
	CHECK_EQUAL(1, seq[1].mInitializeCalled);

	seq[1].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Success);
	CHECK_EQUAL(1, seq[0].mTerminateCalled);
	CHECK_EQUAL(1, seq[1].mTerminateCalled);
}

TEST(BehaviorTrees, Tick_SelectorParentSequenceChildren_SelectorSucceeds)
{
	BehaviorTree bt;
	Selector selector(bt);
	MockSequence sequence1(bt, 2);
	MockSequence sequence2(bt, 2);

	selector.Add(sequence1);
	selector.Add(sequence2);

	bt.Start(selector);
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mInitializeCalled);
	CHECK_EQUAL(0, sequence1[0].mTerminateCalled);

	sequence1[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mTerminateCalled);
	CHECK_EQUAL(1, sequence1[1].mInitializeCalled);
	CHECK_EQUAL(0, sequence1[1].mTerminateCalled);

	sequence1[1].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Success, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mTerminateCalled);
	CHECK_EQUAL(1, sequence1[1].mTerminateCalled);
}


TEST(BehaviorTrees, Tick_SelectorParentSequenceChildren_SelectorSucceedsOnSecondSequence)
{
	BehaviorTree bt;
	Selector selector(bt);
	MockSequence sequence1(bt, 2);
	MockSequence sequence2(bt, 2);

	selector.Add(sequence1);
	selector.Add(sequence2);

	bt.Start(selector);
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mInitializeCalled);
	CHECK_EQUAL(0, sequence1[0].mTerminateCalled);

	sequence1[0].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mTerminateCalled);
	CHECK_EQUAL(0, sequence1[1].mInitializeCalled);
	CHECK_EQUAL(1, sequence2[0].mInitializeCalled);
	
	sequence2[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Running, selector.GetStatus());
	CHECK_EQUAL(1, sequence1[0].mTerminateCalled);
	CHECK_EQUAL(1, sequence2[0].mTerminateCalled);
	CHECK_EQUAL(0, sequence2[1].mTerminateCalled);

	sequence2[1].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Success, selector.GetStatus());
	CHECK_EQUAL(1, sequence2[0].mTerminateCalled);
	CHECK_EQUAL(1, sequence2[1].mTerminateCalled);
}

TEST(BehaviorTrees, Tick_SelectorParentSequenceChildren_SelectorFailsOnSecondSequence)
{
	BehaviorTree bt;
	Selector selector(bt);
	MockSequence sequence1(bt, 2);
	MockSequence sequence2(bt, 2);

	selector.Add(sequence1);
	selector.Add(sequence2);

	bt.Start(selector);
	bt.Tick();

	sequence1[0].mReturnStatus = BehaviorStatus::Failure;

	sequence2[0].mReturnStatus = BehaviorStatus::Success;
	sequence2[1].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();

	CHECK_EQUAL(BehaviorStatus::Failure, selector.GetStatus());
	CHECK_EQUAL(1, sequence2[0].mTerminateCalled);
	CHECK_EQUAL(1, sequence2[1].mTerminateCalled);
}