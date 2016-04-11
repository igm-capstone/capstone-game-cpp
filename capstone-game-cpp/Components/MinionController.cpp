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

MinionController::MinionController()
	: mSpeed(0)
	, mThinkTime(0)
	, mWanderTime(0)
	, mAI(Singleton<AIManager>::SharedInstance())
	, mTimer(*Singleton<Engine>::SharedInstance().GetTimer())
	, mAngle(0)
	, mDirectionIndex(0)
	, mIsTransformDirty(false)
{
	Tree& followExplorer = TreeBuilder("(-->) Follow Explorer")
		.Composite<Sequence>()
			.Conditional()
				.Predicate(&IsExplorerInRange, "(?) Is Explorer in Range")
				.Composite<Parallel>()
					.Action(&MoveTowardsExplorer, "(!) Move Towards Explorer")
					.Action(&LookForward, "(!) Look Forward")
				.End()
			.End()
		.End()
	.End();

	Tree& wanderArround = TreeBuilder("(-->) Wander Arround")
		.Composite<Sequence>()
			.Action(&Think, "(!) Think")
			.Action(&UpdateWanderDirection, "(!) Update Wander Direction")
			.Composite<Parallel>()
				.Action(&MoveForward, "(!) Move Forward")
				.Action(&LookForward, "(!) Look Forward")
			.End()
		.End()
	.End();

	mBehaviorTree = &TreeBuilder()
		.Composite<Priority>("(/!\\) Priority Selector")
			.Subtree(followExplorer)
			.Subtree(wanderArround)
		.End()
	.End();
}


MinionController::~MinionController()
{

}


bool MinionController::Update(double milliseconds)
{
	if (!mIsActive) return false;
	
	mLastPosition = mPosition;
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
		OnMove(mPosition, GetAdjustedRotation(mAngle));
	}

	if (Singleton<Engine>::SharedInstance().GetInput()->GetKeyDown(KEYCODE_D))
	{
		TRACE_LOG(duration);
	}

	return true;
}


quatf MinionController::GetAdjustedRotation(float angle) {
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
		self.mThinkTime = Mathf::RandomRange(1.5f, 4.5f);
		return BehaviorStatus::Success;
	}

	self.mThinkTime -= float(self.mTimer.GetDeltaTime()) * 0.001f;

	self.PlayStateAnimation(ANIM_STATE_IDLE);
	return BehaviorStatus::Running;
}

BehaviorStatus MinionController::MoveForward(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	bool validDirection = false;
	vec2f direction = sDirections[self.mDirectionIndex];
	Node& node = *self.mAI.GetNodeAt(self.mPosition);
	Node& nextNode = self.mAI.mGrid(node.x + int(direction.x), node.y + int(direction.y));

	// check what is ahead and if invalid try to find a valid one (rotating clockwise)
	for (size_t i = 0; i < 8; i++)
	{
		auto nextNodeState = nextNode.GetState();
		if (nextNodeState != Node::BLOCKED && nextNodeState != Node::UNKNOWN)
		{
			// found a valid direction
			validDirection = true;
			break;
		}

		// next direction clockwise
		self.mDirectionIndex = (self.mDirectionIndex + 1) % 8;
		direction = sDirections[self.mDirectionIndex];
		nextNode = self.mAI.mGrid(node.x + int(direction.x), node.y + int(direction.y));
	}

	// if cant find a valid direction, return failure
	if (!validDirection)
	{
		return BehaviorStatus::Failure;
	}

	auto timer = Singleton<Engine>::SharedInstance().GetTimer();
	float dt = float(timer->GetDeltaTime()) * 0.001f;

	// speed per second
	vec2f targetVelocity = normalize(vec2f(nextNode.worldPos) - vec2f(self.mPosition)) * 10 * dt;

	// delta space for the current frame
	vec3f ds = targetVelocity;

	// update position
	self.mPosition += ds;
	self.mIsTransformDirty = true;
	self.PlayStateAnimation(ANIM_STATE_WALK);

	self.mWanderTime -= dt;
	if (self.mWanderTime <= 0)
	{
		// TODO: parametrize wander time
		self.mWanderTime = Mathf::RandomRange(1, 2);
		return BehaviorStatus::Success;
	}

	return BehaviorStatus::Running;
}

BehaviorStatus MinionController::UpdateWanderDirection(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	Node& node = *self.mAI.GetNodeAt(self.mPosition);
	self.mDirectionIndex = Mathf::RandomRangeInt(0, 8);
	
	// try all directions
	for (size_t i = 0; i < 8; i++)
	{
		vec2f direction = sDirections[self.mDirectionIndex];
		
		auto nextNodeState = self.mAI.mGrid(node.x + int(direction.x), node.y + int(direction.y)).GetState();
		if (nextNodeState != Node::BLOCKED && nextNodeState != Node::UNKNOWN)
		{
			// found a valid direction
			return BehaviorStatus::Success;
		}

		// next direction clockwise
		self.mDirectionIndex = (self.mDirectionIndex + 1) % 8;
	}

	return BehaviorStatus::Running;
}

BehaviorStatus MinionController::LookForward(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	float dt = float(self.mTimer.GetDeltaTime()) * 0.001f;

	vec2f dir = normalize(vec2f(self.mPosition) - vec2f(self.mLastPosition));

	float targetAngle = atan2f(dir.y, dir.x);

	if (abs(self.mAngle - targetAngle) < 0.001f) {
		return BehaviorStatus::Success;
	}

	self.mAngle = Mathf::LerpAngle(self.mAngle, targetAngle, 8 * dt);
	self.mIsTransformDirty = true;


	return BehaviorStatus::Success;
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