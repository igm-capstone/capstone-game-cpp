#include "stdafx.h"
#include "MinionController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <BehaviorTree/Action.h>
#include <BehaviorTree/Sequence.h>
#include <Mathf.h>


MinionController::MinionController()
{
	mBehaviorTree = new BehaviorTree();
	auto sequence = new Sequence(*mBehaviorTree);

	auto findTarget = new Action();
	findTarget->mUpdate = std::bind(&MinionController::FindTarget, this);
	sequence->mChildren.push_back(findTarget);

	auto follow = new Action();
	follow->mUpdate = std::bind(&MinionController::MoveTowardsTarget, this);
	sequence->mChildren.push_back(follow);

	mBehaviorTree->Start(*sequence);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	mBehaviorTree->Tick();
	
	return true;
}

BehaviorStatus MinionController::FindTarget()
{
	mTarget = vec3f(rand() % 25, rand() % 25, 0);
	return BehaviorStatus::Success;
}

BehaviorStatus MinionController::MoveTowardsTarget()
{
	vec3f myPos = mSceneObject->mTransform->GetPosition();
	vec3f direction = mTarget - myPos;
	float distanceSquared = magnitudeSquared(direction);

	if (distanceSquared < .25)
	{
		return BehaviorStatus::Success;
	}

	auto timer = Singleton<Engine>::SharedInstance().GetTimer();

	// speed per second
	vec3f targetVelocity = normalize(direction) * 0.01f * timer->GetDeltaTime();

	// delta space for the current frame
	vec3f ds = targetVelocity;

	OnMove(myPos + ds);

	return BehaviorStatus::Running;
}
