#include "stdafx.h"
#include "MinionController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <BehaviorTree/Sequence.h>
#include <AIManager.h>
#include <Mathf.h>

//Sequence* s;

MinionController::MinionController():
	mAI(Singleton<AIManager>::SharedInstance())
{
	mBehaviorTree = new BehaviorTree();
	auto sequence = new Sequence(*mBehaviorTree);
	//s = sequence;
	auto findTarget = new Behavior();
	findTarget->SetUpdateCallback(&FindTarget);
	sequence->Add(*findTarget);

	auto follow = new Behavior();
	follow->SetUpdateCallback(&MoveTowardsTarget);
	sequence->Add(*follow);

	mBehaviorTree->Start(*sequence);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;
	//if (mBehaviorTree->mBehaviors.empty())
	//mBehaviorTree->Start(*s);
	mBehaviorTree->Tick(this);
	
	return true;
}

BehaviorStatus MinionController::FindTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	
	auto node = self.mAI.GetNodeAt(self.mSceneObject->mTransform->GetPosition());
	auto target = self.mAI.mGrid(node->x + rand() % 3 - 1, node->y + rand() % 3 - 1);
	auto nodeState = target.GetState();

	if (nodeState != Node::CLEAN && nodeState != Node::PATH)
	{
		return BehaviorStatus::Failure;
	}

	self.mTarget =target.worldPos;
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
