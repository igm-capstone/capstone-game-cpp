#include "stdafx.h"
#include "Sequence.h"
#include "BehaviorTree.h"

Sequence::Sequence(BehaviorTree& bt) : mBehaviorTree(&bt)
{
}

void Sequence::OnInitialize()
{
	mCurrent = mChildren.begin();
	BehaviorObserver observer = std::bind(&Sequence::OnChildComplete, this, std::placeholders::_1);
	mBehaviorTree->Start(**mCurrent, &observer);
}

void Sequence::OnChildComplete(BehaviorStatus status)
{
	Behavior& child = **mCurrent;

	if (child.mStatus == BehaviorStatus::Failure)
	{
		mBehaviorTree->Stop(*this, BehaviorStatus::Failure);
		return;
	}

	ASSERT(child.mStatus == BehaviorStatus::Success);
	if(++mCurrent == mChildren.end())
	{
		mBehaviorTree->Stop(*this, BehaviorStatus::Success);
		mStatus = BehaviorStatus::Invalid;
	}
	else
	{
		BehaviorObserver observer = std::bind(&Sequence::OnChildComplete, this, std::placeholders::_1);
		mBehaviorTree->Start(**mCurrent, &observer);
	}

}

BehaviorStatus Sequence::Update()
{
	return BehaviorStatus::Running;
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
		CHECK_EQUAL(seq.mStatus, BehaviorStatus::Running);
		CHECK_EQUAL(0, seq[0].mTerminateCalled);

		seq[0].mReturnStatus = status[i];
		bt.Tick();
		CHECK_EQUAL(seq.mStatus, status[i]);
		CHECK_EQUAL(1, seq[0].mTerminateCalled);
	}
}

TEST(BehaviorTrees, SequenceTwoFails)
{
	BehaviorTree bt;
	MockSequence seq(bt, 2);

	bt.Start(seq);
	bt.Tick();
	CHECK_EQUAL(seq.mStatus, BehaviorStatus::Running);
	CHECK_EQUAL(0, seq[0].mTerminateCalled);

	seq[0].mReturnStatus = BehaviorStatus::Failure;
	bt.Tick();
	CHECK_EQUAL(seq.mStatus, BehaviorStatus::Failure);
	CHECK_EQUAL(1, seq[0].mTerminateCalled);
}

TEST(BehaviorTrees, SequenceTwoContinues)
{
	BehaviorTree bt;
	MockSequence seq(bt, 2);

	bt.Start(seq);
	bt.Tick();
	CHECK_EQUAL(seq.mStatus, BehaviorStatus::Running);
	CHECK_EQUAL(0, seq[0].mTerminateCalled);

	seq[0].mReturnStatus = BehaviorStatus::Success;
	bt.Tick();
	CHECK_EQUAL(seq.mStatus, BehaviorStatus::Running);
	CHECK_EQUAL(1, seq[0].mTerminateCalled);
}

