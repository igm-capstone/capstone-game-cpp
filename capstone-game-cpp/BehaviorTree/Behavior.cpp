#include "stdafx.h"
#include "Behavior.h"

using namespace BehaviorTree;


Behavior::Behavior(Tree& tree, std::string name) :
	mTree(tree),
	mName(name),
	mStatus(BehaviorStatus::Invalid),
	mObserver(BehaviorObserver::Default()),
	mOnUpdate(nullptr),
	mOnReset(nullptr),
	mOnInitialize(nullptr),
	mOnTerminate(nullptr)
{
	mDumpCallback = [&](std::stringstream& ss, int level)
	{
		ss << "[" << mStatus << "] " << std::string(level, '\t') << mName << std::endl;
	};
}

Behavior::~Behavior()
{
}

BehaviorStatus Behavior::Tick(void* userData)
{
	if (mOnInitialize && mStatus == BehaviorStatus::Invalid)
	{
		mOnInitialize(*this, userData);
	}

	if (mOnUpdate)
	{
		mStatus = mOnUpdate(*this, userData);
	}

	if (mOnTerminate && mStatus != BehaviorStatus::Running)
	{
		mOnTerminate(*this, userData, mStatus);
	}

	return mStatus;
}

void Behavior::Reset(void* data)
{
	if (mOnReset && mStatus != BehaviorStatus::Invalid)
	{
		mOnReset(*this, data);
	}
}
