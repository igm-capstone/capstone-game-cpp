#include "stdafx.h"
#include "BehaviorTree.h"

void BehaviorTree::Start(Behavior& bh, BehaviorObserver* observer)
{
	if (observer != nullptr)
	{
		bh.mObserver = *observer;
	}

	mBehaviors.push_front(&bh);
}

void BehaviorTree::Stop(Behavior& bh, BehaviorStatus result)
{
	ASSERT(result != BehaviorStatus::Running);
	bh.mStatus = result;

	if (bh.mObserver)
	{
		bh.mObserver(result);
	}
}

void BehaviorTree::Tick()
{
	mBehaviors.push_back(nullptr);

	while(Step())
	{
		continue;
	}
}

bool BehaviorTree::Step()
{
	Behavior* current = mBehaviors.front();
	mBehaviors.pop_front();

	if (current == nullptr)
	{
		return false;
	}

	current->Tick();

	if (current->mStatus != BehaviorStatus::Running && current->mObserver)
	{
		current->mObserver(current->mStatus);
	}
	else
	{
		mBehaviors.push_back(current);
	}

	return true;
}
