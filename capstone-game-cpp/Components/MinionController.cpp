#include "stdafx.h"
#include "MinionController.h"
#include <BehaviorTree/BehaviorTree.h>
#include <Rig3D\Common\Timer.h>
#include <Rig3D\Engine.h>
#include <AIManager.h>
#include <SceneObjects/Minion.h>
#include <Components/ColliderComponent.h>
#include <Mathf.h>
#include <SceneObjects/Minion.h>
#include "AnimationController.h"

using namespace BehaviorTree;
using namespace chrono;

MinionController::MinionController():
	mThinkTime(0),
	mAI(Singleton<AIManager>::SharedInstance()),
	mTimer(*Singleton<Engine>::SharedInstance().GetTimer())
{
	mBehaviorTree = new Tree();

	auto baseSelector = new Priority(*mBehaviorTree, "(/!\\) Priority Selector");
	auto followExplorerSequence = new Sequence(*mBehaviorTree, "(-->) Follow Explorer");
	auto isExplorerInRange = new Predicate(*mBehaviorTree, "(?) Is Explorer in Range");
	auto moveTowardsExplorer = new Behavior(*mBehaviorTree, "(!) Move Towards Explorer");
	auto tryToFollowExplorer = new Conditional(*mBehaviorTree, *moveTowardsExplorer, *isExplorerInRange, "(?) Try to Follow Explorer");
	auto patrolSequence = new Sequence(*mBehaviorTree, "(-->) Patrol");
	auto findTarget = new Behavior(*mBehaviorTree, "(!) Find Patrol Target");
	auto think = new Behavior(*mBehaviorTree, "(!) Think");
	auto moveTowardsTarget = new Behavior(*mBehaviorTree, "(!) Move Towards Target");


	isExplorerInRange->SetPredicateCallback(&IsExplorerInRange);
	moveTowardsExplorer->SetUpdateCallback(&MoveTowardsExplorer);
	findTarget->SetUpdateCallback(&FindTarget);
	think->SetUpdateCallback(&Think);
	moveTowardsTarget->SetUpdateCallback(&MoveTowardsTarget);


	baseSelector->Add(*followExplorerSequence);
	baseSelector->Add(*patrolSequence);

	followExplorerSequence->Add(*tryToFollowExplorer);
	//followExplorerSequence->Add(*isExplorerInRange);
	//followExplorerSequence->Add(*moveTowardsExplorer);

	patrolSequence->Add(*findTarget);
	patrolSequence->Add(*think);
	patrolSequence->Add(*moveTowardsTarget);


	mBehaviorTree->Start(*baseSelector);

	quatf rot;
	UpdateRotation(0, rot);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;
	
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	mBehaviorTree->Tick(this);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = duration_cast<nanoseconds>(t2 - t1).count();
	
	auto pos = mSceneObject->mTransform->GetPosition();
	auto dir = mSceneObject->mTransform->GetUp();
	TRACE_LINE(pos, pos + dir, Colors::red);

	if (Singleton<Engine>::SharedInstance().GetInput()->GetKeyDown(KEYCODE_D))
	{
		TRACE_LOG(duration);
	}

	return true;
}


bool MinionController::UpdateRotation(float angle, quatf& rot) {
	quatf newRot = normalize(quatf::angleAxis(angle, vec3f(0, 0, 1)) * quatf::rollPitchYaw(-0.5f * PI, 0, 0) * quatf::rollPitchYaw(0, -0.5f * PI, 0));
	auto hasRotated = rot != newRot;
	rot = newRot;
	return hasRotated;
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
	vec2f direction = vec2f(targetConn.to->worldPos) - vec2f(myPos);
	
	float distanceSquared = magnitudeSquared(direction);

	if (distanceSquared < .25)
	{
		return BehaviorStatus::Success;
	}

	auto timer = Singleton<Engine>::SharedInstance().GetTimer();

	// speed per second
	vec2f targetVelocity = normalize(direction) * 0.01f * static_cast<float>(timer->GetDeltaTime());

	// delta space for the current frame
	vec3f ds = targetVelocity;

	self.OnMove(myPos + ds);

	self.PlayStateAnimation(ANIM_STATE_WALK);
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

	self.PlayStateAnimation(ANIM_STATE_IDLE);
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
	vec2f direction = self.mTarget - vec2f(myPos);
	float distanceSquared = magnitudeSquared(direction);

	if (distanceSquared < .25)
	{
		return BehaviorStatus::Success;
	}

	auto timer = Singleton<Engine>::SharedInstance().GetTimer();

	// speed per second
	vec2f targetVelocity = normalize(direction) * 0.01f * static_cast<float>(timer->GetDeltaTime());

	// delta space for the current frame
	vec3f ds = targetVelocity;

	self.OnMove(myPos + ds);

	self.PlayStateAnimation(ANIM_STATE_WALK);
	return BehaviorStatus::Running;
}

void MinionController::OnMeleeStart(void* obj)
{
	auto minion = reinterpret_cast<Minion*>(obj);
	minion->mMeleeColliderComponent->mIsActive = true;
}

void MinionController::OnMeleeStop(void* obj)
{
	auto minion = reinterpret_cast<Minion*>(obj);
	minion->mMeleeColliderComponent->mIsActive = false;
}

void MinionController::OnMeleeHit(BaseSceneObject* minion, BaseSceneObject* other)
{

}


void MinionController::PlayStateAnimation(AnimationControllerState state)
{
	AnimationController* pAnimationController = reinterpret_cast<Minion*>(mSceneObject)->mAnimationController;
	pAnimationController->SetState(state);
	pAnimationController->Resume();
}

void MinionController::PauseStateAnimation(AnimationControllerState state)
{
	AnimationController* pAnimationController = reinterpret_cast<Minion*>(mSceneObject)->mAnimationController;
	if (pAnimationController->GetState() == state)
	{
		pAnimationController->Pause();
	}
}