#include "stdafx.h"
#include "FlyTrapController.h"
#include "BehaviorTree/BehaviorTree.h"

using namespace BehaviorTree;

FlyTrapController::FlyTrapController()
{
	mBaseRotation = quatf::rollPitchYaw(PI, 0, 0) * quatf::rollPitchYaw(0, -0.5f * PI, 0);
	
	mBehaviorTree = &TreeBuilder(mAllocator)
		.Composite<Priority>("(/!\\) Priority Selector")
			.Subtree(CreateAttackSubtree())
		.End()
	.End();
}


FlyTrapController::~FlyTrapController()
{
}