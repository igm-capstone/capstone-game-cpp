#include "stdafx.h"
#include "MinionController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <BehaviorTree/Sequence.h>
#include <BehaviorTree/Condition.h>
#include <BehaviorTree/PrioritySelector.h>
#include <Rig3D\Common\Timer.h>
#include <Rig3D\Engine.h>
#include <AIManager.h>
#include <Mathf.h>

MinionController::MinionController():
	mThinkTime(0),
	mAI(Singleton<AIManager>::SharedInstance()),
	mTimer(*(Singleton<Engine>::SharedInstance()).GetTimer())
{
	mBehaviorTree = new BehaviorTree();

	auto baseSelector = new PrioritySelector(*mBehaviorTree, "(/!\\) Priority Selector");
	auto followExplorerSequence = new Sequence(*mBehaviorTree, "(-->) Follow Explorer");
	auto isExplorerInRange = new Condition("(?) Is Explorer in Range");
	auto moveTowardsExplorer = new Behavior("(!) Move Towards Explorer");
	auto patrolSequence = new Sequence(*mBehaviorTree, "(-->) Patrol");
	auto findTarget = new Behavior("(!) Find Patrol Target");
	auto think = new Behavior("(!) Think");
	auto moveTowardsTarget = new Behavior("(!) Move Towards Target");


	isExplorerInRange->SetConditionCallback(&IsExplorerInRange);
	moveTowardsExplorer->SetUpdateCallback(&MoveTowardsExplorer);
	findTarget->SetUpdateCallback(&FindTarget);
	think->SetUpdateCallback(&Think);
	moveTowardsTarget->SetUpdateCallback(&MoveTowardsTarget);


	baseSelector->Add(*followExplorerSequence);
	baseSelector->Add(*patrolSequence);

	followExplorerSequence->Add(*isExplorerInRange);
	followExplorerSequence->Add(*moveTowardsExplorer);

	patrolSequence->Add(*findTarget);
	patrolSequence->Add(*think);
	patrolSequence->Add(*moveTowardsTarget);


	mBehaviorTree->Start(*baseSelector);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;
	
	mBehaviorTree->Tick(this);
	
	auto pos = mSceneObject->mTransform->GetPosition();
	auto dir = mSceneObject->mTransform->GetUp();
	TRACE_LINE(pos, pos + dir, Colors::red);

	return true;
}


bool MinionController::IsExplorerInRange(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	
	auto node = self.mAI.GetNodeAt(self.mSceneObject->mTransform->GetPosition());
	auto nodeState = node->GetState();

	return nodeState == Node::PATH;
}

BehaviorStatus MinionController::MoveTowardsExplorer(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	vec3f myPos = self.mSceneObject->mTransform->GetPosition();
	auto myNode = self.mAI.GetNodeAt(myPos);

	auto targetConn = self.mAI.mGrid.GetBestFitConnection(myNode);
	vec3f direction = targetConn.to->worldPos - myPos;
	
	float distanceSquared = magnitudeSquared(direction);

	if (distanceSquared < .25)
	{
		return BehaviorStatus::Success;
	}

	auto timer = Singleton<Engine>::SharedInstance().GetTimer();

	// speed per second
	vec3f targetVelocity = normalize(direction) * 0.01f * static_cast<float>(timer->GetDeltaTime());

	// delta space for the current frame
	vec3f ds = targetVelocity;

	self.OnMove(myPos + ds);

	return BehaviorStatus::Running;
}

BehaviorStatus MinionController::Think(Behavior& bh, void* data) {

	auto& self = *static_cast<MinionController*>(data);

	if (self.mThinkTime <= 0) {
		self.mThinkTime = 0.5f + rand() % 3;
		return BehaviorStatus::Success;
	}

	auto dt = static_cast<float>(self.mTimer.GetDeltaTime()) * 0.001f;
	self.mThinkTime = max(0, self.mThinkTime - dt);

	return BehaviorStatus::Running;
}

BehaviorStatus MinionController::FindTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	
	auto node = self.mAI.GetNodeAt(self.mSceneObject->mTransform->GetPosition());
	auto target = self.mAI.mGrid(node->x + rand() % 9 - 4, node->y + rand() % 9 - 4);
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
	vec3f targetVelocity = normalize(direction) * 0.01f * static_cast<float>(timer->GetDeltaTime());

	// delta space for the current frame
	vec3f ds = targetVelocity;

	self.OnMove(myPos + ds);

	return BehaviorStatus::Running;
}
