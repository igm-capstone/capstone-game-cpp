#include "stdafx.h"
#include "ImpController.h"
#include "BehaviorTree/BehaviorTree.h"

using namespace BehaviorTree;

ImpController::ImpController() 
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


ImpController::~ImpController()
{
}
