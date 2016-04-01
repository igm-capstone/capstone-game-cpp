#include "stdafx.h"
#include "Tree.h"
#include <trace.h>

using namespace BehaviorTree;

Tree::Tree(std::string name) : Behavior(*this, name), mRootBehavior(nullptr)
{
	SetUpdateCallback(OnUpdate);

	mDumpCallback = [&](std::stringstream& ss, int level)
	{
		ss << "[" << mStatus << "] " << std::string(level, '\t') << mName << std::endl;
		mRootBehavior->Dump(ss, level + 1);
	};
}

void Tree::Start(Behavior& bh)
{
	mRootBehavior = &bh;
}

BehaviorStatus Tree::OnUpdate(Behavior& bh, void* userData)
{
	auto& self = reinterpret_cast<Tree&>(bh);
	
	return self.mRootBehavior->Tick(userData);
}