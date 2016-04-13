#include "stdafx.h"
#include "FlyTrapController.h"
#include "BehaviorTree/BehaviorTree.h"

using namespace BehaviorTree;

FlyTrapController::FlyTrapController()
{
		Tree& attackExplorer = TreeBuilder("(-->) Attack")
		.Composite<Sequence>()
			.Decorator<Mute>()
				.Conditional()
					.Predicate(&IsExplorerInAttackRange, "(?) Is Explorer in Attack Range")
					.Action(&StartAttack, "Start Attack")
				.End()
			.End()
			// look at player ;)
			.Predicate(&IsAttackInProgress, "(?) Is Attack in Progress")
		.End()
	.End();

	mBehaviorTree = &TreeBuilder()
		.Composite<Priority>("(/!\\) Priority Selector")
			.Subtree(attackExplorer)
		.End()
	.End();
}


FlyTrapController::~FlyTrapController()
{
}
