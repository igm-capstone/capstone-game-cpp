#include "stdafx.h"
#include "Sequence.h"
#include "BehaviorTree.h"

Sequence::Sequence(BehaviorTree& bt) : mBehaviorTree(&bt)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);
}

void Sequence::OnInitialize(Behavior& bh, void* data)
{
	auto& self = static_cast<Sequence&>(bh);

	self.mCurrent = self.mChildren.begin();
	self.mBehaviorTree->Start(**self.mCurrent, &OnChildComplete, &self);
}

void Sequence::OnChildComplete(void* observerData, BehaviorStatus status)
{
	auto& self = *static_cast<Sequence*>(observerData);

	const Behavior& child = **self.mCurrent;

	if (child.GetStatus() == BehaviorStatus::Failure)
	{
		self.mBehaviorTree->Stop(self, BehaviorStatus::Failure);
		return;
	}

	ASSERT(child.GetStatus() == BehaviorStatus::Success);
	if(++self.mCurrent == self.mChildren.end())
	{
		self.mBehaviorTree->Stop(self, BehaviorStatus::Success);
		//self.mStatus = BehaviorStatus::Invalid;
	}
	else
	{
		//BehaviorObserver observer = std::bind(&Sequence::OnChildComplete, this, std::placeholders::_1);
		//mBehaviorTree->Start(**mCurrent, &observer);
		self.mBehaviorTree->Start(**self.mCurrent, &OnChildComplete, &self);
	}

}

BehaviorStatus Sequence::OnUpdate(Behavior& self, void* data)
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
		CHECK_EQUAL(seq.GetStatus(), BehaviorStatus::Running);
		CHECK_EQUAL(0, seq[0].mTerminateCalled);

		seq[0].mReturnStatus = status[i];
		bt.Tick();
		CHECK_EQUAL(seq.GetStatus(), status[i]);
		CHECK_EQUAL(1, seq[0].mTerminateCalled);
	}
}

TEST(BehaviorTrees, SequenceTwoFails)
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
}

TEST(BehaviorTrees, SequenceTwoContinues)
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
}

