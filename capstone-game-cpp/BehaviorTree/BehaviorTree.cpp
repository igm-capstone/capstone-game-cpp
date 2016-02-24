#include "stdafx.h"
#include "BehaviorTree.h"

void BehaviorTree::Start(Behavior& bh, BehaviorObserver observer)
{
	bh.SetObserver(observer);
	Start(bh);
}

void BehaviorTree::Start(Behavior& bh)
{
	mBehaviors.push_front(&bh);
}

void BehaviorTree::Stop(Behavior& bh, BehaviorStatus result)
{
	ASSERT(result != BehaviorStatus::Running);

	bh.mStatus = result;
	bh.NotifyObserver(result);
}

void BehaviorTree::Tick(void* userData)
{
	// placeholder for end of frame
	mBehaviors.push_back(nullptr);

	// keep stepping until nullptr placeholder is found
	while (Step(userData))
	{
		continue;
	}
}

bool BehaviorTree::Step(void* userData)
{
	Behavior* current = mBehaviors.front();
	mBehaviors.pop_front();

	// if popped behavior is nullptr, return false (end of frame)
	if (!current)
	{
		return false;
	}

	// we could use suspended status here
	current->Tick(userData);

	if (current->mStatus != BehaviorStatus::Running)
	{
		current->NotifyObserver(current->mStatus);
	}
	else
	{
		mBehaviors.push_back(current);
	}

	return true;
}
