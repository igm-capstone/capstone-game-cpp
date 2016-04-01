#include "stdafx.h"
#include "Conditional.h"
#include "Predicate.h"
#include "Tree.h"

using namespace BehaviorTree;

Conditional::Conditional(Tree& tree, Behavior& child, Predicate& predicate, std::string name)
	: Decorator(tree, child, name)
	, mPredicate(predicate)
{
	SetInitializeCallback(&OnInitialize);
	SetUpdateCallback(&OnUpdate);

	mDumpCallback = [&](std::stringstream& ss, int level)
	{
		ss << "[" << mStatus << "] " << std::string(level, '\t') << mName << std::endl;
		mPredicate.Dump(ss, level + 1);
		mChild.Dump(ss, level + 1);
	};
}

void Conditional::OnInitialize(Behavior& bh, void* data)
{
	//auto& self = static_cast<Conditional&>(bh);

	//self.mTree.Start(self.mChild, { &OnChildComplete, &self, data });
}

BehaviorStatus Conditional::OnUpdate(Behavior& bh, void* data)
{
	auto& self = reinterpret_cast<Conditional&>(bh);

	self.mPredicate.Tick(data);

	if (!self.mPredicate.IsValid())
	{
		return BehaviorStatus::Failure;
	}
	
	return self.mChild.Tick(data);
}