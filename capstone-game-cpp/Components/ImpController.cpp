#include "stdafx.h"
#include "ImpController.h"
#include "BehaviorTree/BehaviorTree.h"

using namespace BehaviorTree;

ImpController::ImpController() 
	: MinionController()
{
	Tree& attackExplorer = TreeBuilder("(-->) Attack")
		.Composite<Sequence>()
			.Decorator<Mute>()
				.Conditional()
					.Predicate(&IsExplorerInAttackRange, "(?) Is Explorer in Attack Range")
					.Action(&StartAttack, "Start Attack")
				.End()
			.End()
			.Predicate(&IsAttackInProgress, "(?) Is Attack in Progress")
		.End()
	.End();

	mBehaviorTree = &TreeBuilder()
		.Composite<Priority>("(/!\\) Priority Selector")
			.Subtree(attackExplorer)
			.Subtree(CreateChaseSubtree())
			.Subtree(CreateWanderSubtree())
		.End()
	.End();
}


ImpController::~ImpController()
{
}
