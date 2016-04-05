#include "stdafx.h"
#include "Predicate.h"

using namespace BehaviorTree;

Predicate::Predicate(Tree& tree, std::string name) : Behavior(tree, name)
{
	SetUpdateCallback(&OnUpdate);
}

BehaviorStatus Predicate::OnUpdate(Behavior& bh, void* data)
{
	auto& self = static_cast<Predicate&>(bh);

	if (!self.mOnPredicate || self.mOnPredicate(bh, data))
	{
		return BehaviorStatus::Success;
	}

	return BehaviorStatus::Failure;
}
