#include "stdafx.h"
#include "AbominationController.h"
#include "BehaviorTree/BehaviorTree.h"

using namespace BehaviorTree;

AbominationController::AbominationController()
	: MinionController()
{
	mBehaviorTree = &TreeBuilder(mAllocator)
		.Composite<Priority>("(/!\\) Priority Selector")
		.Subtree(CreateKnockbackSubtree())
		.Subtree(CreateAttackSubtree())
		.Subtree(CreateChaseSubtree())
		.Subtree(CreateWanderSubtree())
		.End()
		.End();
}


AbominationController::~AbominationController()
{
}
