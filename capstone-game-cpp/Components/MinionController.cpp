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
#include <BehaviorTree/Parallel.h>
#include <BehaviorTree/Builder.h>

using namespace BehaviorTree;
using namespace chrono;
using namespace cliqCity::graphicsMath;

// unnormalized direction vectors, going clockwise
const vec2f MinionController::sDirections[] = {
	{  1,  0 }, {  1, -1 }, {  0, -1 }, { -1, -1 },
	{ -1,  0 }, { -1,  1 }, {  0,  1 }, {  1,  1 },
};

MinionController::MinionController():
	mThinkTime(0),
	mAI(Singleton<AIManager>::SharedInstance()),
	mTimer(*Singleton<Engine>::SharedInstance().GetTimer())
{

	Tree* wander = &TreeBuilder()
		.End();

	mBehaviorTree = &TreeBuilder()
		.Composite<Priority>("(/!\\) Priority Selector")
			.Composite<Sequence>("(-->) Follow Explorer")
				.Conditional()
					.Predicate(&IsExplorerInRange, "(?) Is Explorer in Range")
					.Action(&MoveTowardsExplorer, "(!) Move Towards Explorer")
				.End()
			.End()
			.Composite<Sequence>("(-->) Wander Arround")
				.Action(&Think, "(!) Think")
				.Action(&FindTarget, "(!) Find Patrol Target")
				.Composite<Parallel>()
					.Action(&RotateTowardsTarget, "(!) Rotate Towards Target")
					.Action(&MoveTowardsTarget, "(!) Move Towards Target")
				.End()
			.End()
		.End()
	.End();

	return;
	mBehaviorTree = new Tree();

	auto baseSelector = new Priority(*mBehaviorTree, "(/!\\) Priority Selector");
	auto followExplorerSequence = new Sequence(*mBehaviorTree, "(-->) Follow Explorer");
	auto isExplorerInRange = new Predicate(*mBehaviorTree, "(?) Is Explorer in Range");
	auto moveTowardsExplorer = new Behavior(*mBehaviorTree, "(!) Move Towards Explorer");
	auto tryToFollowExplorer = new Conditional(*mBehaviorTree, moveTowardsExplorer, isExplorerInRange, "(?) Try to Follow Explorer");
	auto patrolSequence = new Sequence(*mBehaviorTree, "(-->) Patrol");
	auto findTarget = new Behavior(*mBehaviorTree, "(!) Find Patrol Target");
	auto think = new Behavior(*mBehaviorTree, "(!) Think");
	auto followTarget = new Parallel(*mBehaviorTree, "(!!) Follow Target");
	auto rotateTowardsTarget = new Behavior(*mBehaviorTree, "(!) Rotate Towards Target");
	auto moveTowardsTarget = new Behavior(*mBehaviorTree, "(!) Move Towards Target");


	isExplorerInRange->SetPredicateCallback(&IsExplorerInRange);
	moveTowardsExplorer->SetUpdateCallback(&MoveTowardsExplorer);
	findTarget->SetUpdateCallback(&FindTarget);
	think->SetUpdateCallback(&Think);
	moveTowardsTarget->SetUpdateCallback(&MoveTowardsTarget);
	rotateTowardsTarget->SetUpdateCallback(&RotateTowardsTarget);

	baseSelector->Add(*followExplorerSequence);
	baseSelector->Add(*patrolSequence);

	followExplorerSequence->Add(*tryToFollowExplorer);
	//followExplorerSequence->Add(*isExplorerInRange);
	//followExplorerSequence->Add(*moveTowardsExplorer);

	followTarget->Add(*moveTowardsTarget);
	followTarget->Add(*rotateTowardsTarget);

	patrolSequence->Add(*findTarget);
	patrolSequence->Add(*think);
	patrolSequence->Add(*followTarget);


	mBehaviorTree->Start(*baseSelector);
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;
	
	mPosition = mSceneObject->mTransform->GetPosition();
	//mAngle = mSceneObject->mTransform->GetRollPitchYaw().x;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	mBehaviorTree->Tick(this);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = duration_cast<nanoseconds>(t2 - t1).count();
	
	//auto pos = mSceneObject->mTransform->GetPosition();
	//auto dir = mSceneObject->mTransform->GetForward();
	//TRACE_LINE(pos, pos + dir, Colors::red);

	if (mIsTransformDirty)
	{
		OnMove(mPosition, GetRotation(mAngle));
	}

	if (Singleton<Engine>::SharedInstance().GetInput()->GetKeyDown(KEYCODE_D))
	{
		TRACE_LOG(duration);
	}

	return true;
}


quatf MinionController::GetRotation(float angle) {
	return normalize(quatf::angleAxis(angle, vec3f(0, 0, 1)) * quatf::rollPitchYaw(-0.5f * PI, 0, 0) * quatf::rollPitchYaw(0, -0.5f * PI, 0));
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

	auto myNode = self.mAI.GetNodeAt(self.mPosition);

	auto targetConn = self.mAI.mGrid.GetBestFitConnection(myNode);
	vec2f direction = vec2f(targetConn.to->worldPos) - vec2f(self.mPosition);
	
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

	self.mPosition += ds;
	self.mIsTransformDirty = true;

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
	
	Node& node = *self.mAI.GetNodeAt(self.mSceneObject->mTransform->GetPosition());
	Node& target = self.mAI.mGrid(node.x + rand() % 9 - 4, node.y + rand() % 9 - 4);
	auto nodeState = target.GetState();

	if (nodeState != Node::CLEAN && nodeState != Node::PATH)
	{
		return BehaviorStatus::Failure;
	}

	self.mTarget = target.worldPos;

	return BehaviorStatus::Success;
}


BehaviorStatus MinionController::RotateTowardsTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	float dt = float(self.mTimer.GetDeltaTime()) * 0.001f;

	vec2f dir = normalize(self.mTarget - vec2f(self.mPosition));

	//TRACE_LINE(lastPos, lastPos + dir * 5, Colors::red);

	float targetAngle = atan2f(dir.y, dir.x);


	if (abs(self.mAngle - targetAngle) < 0.001f) {
		return BehaviorStatus::Success;
	}

	self.mAngle = Mathf::LerpAngle(self.mAngle, targetAngle, 8 * dt);
	self.mIsTransformDirty = true;


	return BehaviorStatus::Success;
}

BehaviorStatus MinionController::MoveTowardsTarget(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	vec2f direction = self.mTarget - vec2f(self.mPosition);
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

	self.mPosition += ds;
	self.mIsTransformDirty = true;

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