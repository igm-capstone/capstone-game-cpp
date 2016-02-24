#include "stdafx.h"
#include "MinionController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <BehaviorTree/Sequence.h>
#include <Mathf.h>


MinionController::MinionController()
{
	mBehaviorTree = new BehaviorTree();
	auto sequence = new Sequence(*mBehaviorTree);

	auto findTarget = new Behavior();
	findTarget->SetUpdateCallback(&FindTarget);
	sequence->mChildren.push_back(findTarget);

	auto follow = new Behavior();
	follow->SetUpdateCallback(&MoveTowardsTarget);
	sequence->mChildren.push_back(follow);

	mBehaviorTree->Start(*sequence);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;

	mBehaviorTree->Tick(this);
	
	return true;
}

BehaviorStatus MinionController::FindTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	self.mTarget = vec3f(rand() % 25, rand() % 25, 0);
	return BehaviorStatus::Success;
}

BehaviorStatus MinionController::MoveTowardsTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	vec3f myPos = self.mSceneObject->mTransform->GetPosition();
	vec3f direction = self.mTarget - myPos;
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

	self.OnMove(myPos + ds);

	return BehaviorStatus::Running;
}
