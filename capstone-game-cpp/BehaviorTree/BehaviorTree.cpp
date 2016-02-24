#include "stdafx.h"
#include "BehaviorTree.h"

void BehaviorTree::Start(Behavior& bh, ObserverCallback observer, void* data)
{
	bh.SetObserver(observer, data);
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
	mBehaviors.push_back(nullptr);

	while (Step(userData))
	{
		continue;
	}
}

bool BehaviorTree::Step(void* userData)
{
	Behavior* current = mBehaviors.front();
	mBehaviors.pop_front();

	if (!current)
	{
		return false;
	}

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
