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
#include <SceneObjects/Explorer.h>
#include "Health.h"

using namespace BehaviorTree;
using namespace chrono;
using namespace cliqCity::graphicsMath;

// unnormalized direction vectors, going clockwise
const vec2f MinionController::sDirections[] = {
	{  1,  0 }, {  1, -1 }, {  0, -1 }, { -1, -1 },
	{ -1,  0 }, { -1,  1 }, {  0,  1 }, {  1,  1 },
};

MinionController::MinionController()
	: mAllocator(10240) // TODO, tweak allocator size
	, mAI(Singleton<AIManager>::SharedInstance())
	, mTimer(*Singleton<Engine>::SharedInstance().GetTimer())
	, mBehaviorTree(nullptr)
	, mBaseRotation(quatf::rollPitchYaw(-0.5f * PI, 0, 0) * quatf::rollPitchYaw(0, -0.5f * PI, 0))
	, mBaseMoveSpeed(0)
	, mAttackDamage(0)
	, mAttackRange(0)
	, mTurnRate(0)
	, mStunOnHitDuration(0)
	, mSplashRange(0)
	, mSpeed(0)
	, mThinkTime(0)
	, mWanderTime(0)
	, mAngle(0)
	, mDirectionIndex(0)
	, mIsTransformDirty(false)
{

}


MinionController::~MinionController()
{
	mAllocator.Free(); // TODO check average used memory from allocator to decide final allocator size
}

Tree& MinionController::CreateAttackSubtree()
{
	return TreeBuilder(mAllocator, "(-->) Attack")
		.Composite<Parallel>()
			.Decorator<Mute>()
				.Conditional()
					.Predicate(&IsExplorerInAttackRange, "(?) Is Explorer in Attack Range")
					.Action(&StartAttack, "(!) Start Attack")
				.End()
			.End()
			.Conditional()
				.Predicate(&IsAttackInProgress, "(?) Is Attack in Progress")
				.Action(&TargetClosestExplorer, "(!) Target Explorer")
				.Action(&LookAtTarget, "(!) Look at Target")
			.End()
		.End()
	.End();
}

Tree& MinionController::CreateWanderSubtree()
{
	return TreeBuilder(mAllocator, "(-->) Wander Arround")
		.Composite<Sequence>()
			.Action(&Think, "(!) Think")
			.Action(&UpdateWanderDirection, "(!) Update Wander Direction")
			.Composite<Parallel>()
				.Action(&MoveForward, "(!) Move Forward")
				.Action(&LookForward, "(!) Look Forward")
			.End()
		.End()
	.End();
}

Tree& MinionController::CreateChaseSubtree()
{
	return TreeBuilder(mAllocator, "(-->) Follow Explorer")
		.Composite<Sequence>()
			.Conditional()
				.Predicate(&IsExplorerVisible, "(?) Is Explorer Visible")
				.Composite<Parallel>()
					.Action(&MoveTowardsExplorer, "(!) Move Towards Explorer")
					.Action(&LookForward, "(!) Look Forward")
				.End()
			.End()
		.End()
	.End();
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

bool MinionController::IsExplorerInRange(int steps, bool setTarget)
{
	auto node = mAI.GetNodeAt(mSceneObject->mTransform->GetPosition());
	auto conn = mAI.mGrid.GetBestFitConnection(node);

	// when no explorer is arround, BestFitConnection will return null
	if (conn.to == nullptr)
	{
		return false;
	}

	while (steps-- > 0)
	{
		// explorer is at the goal node
		if (conn.to->GetState() == Node::GOAL)
		{
			if (setTarget)
			{
				mTarget = conn.to->worldPos;
			}
			return true;
		}

		node = conn.to;
		conn = mAI.mGrid.GetBestFitConnection(node);
	}

	// explorer is too many steps away
	return false;
}

bool MinionController::LookAt(vec2f dir)
{
	float dt = float(mTimer.GetDeltaTime()) * 0.001f;

	float targetAngle = atan2f(dir.y, dir.x);

	if (abs(mAngle - targetAngle) < 0.001f) {
		return true;
	}

	mAngle = Mathf::LerpAngle(mAngle, targetAngle, 8 * dt);
	mIsTransformDirty = true;

	return false;
}

quatf MinionController::GetAdjustedRotation(float angle)
{
	return normalize(quatf::angleAxis(angle, vec3f(0, 0, 1)) * mBaseRotation);
}

bool MinionController::IsExplorerInAttackRange(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	return self.IsExplorerInRange(3);
}

bool MinionController::IsExplorerInLockRange(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	return self.IsExplorerInRange(10);
}

bool MinionController::IsAttackInProgress(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	auto& minion = *static_cast<Minion*>(self.mSceneObject);

	return minion.mAnimationController->GetState() == ANIM_STATE_MELEE;
}

bool MinionController::IsExplorerVisible(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	
	auto node = self.mAI.GetNodeAt(self.mSceneObject->mTransform->GetPosition());
	auto conn = self.mAI.mGrid.GetBestFitConnection(node);

	if (conn.to == nullptr)
	{
		return false;
	}

	auto state = conn.to->GetState();

	return state == Node::PATH;
}

BehaviorStatus MinionController::StartAttack(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	
	self.PlayStateAnimation(ANIM_STATE_MELEE);

	return BehaviorStatus::Success;
}

BehaviorStatus MinionController::TargetClosestExplorer(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);
	self.IsExplorerInRange(50);

	return BehaviorStatus::Success;
}

BehaviorStatus MinionController::MoveTowardsExplorer(Behavior& bh, void* data)
{
	auto& self = *static_cast<MinionController*>(data);

	auto myNode = self.mAI.GetNodeAt(self.mPosition);

	auto targetConn = self.mAI.mGrid.GetBestFitConnection(myNode);
	vec2f direction = vec2f(targetConn.to->worldPos) - vec2f(self.mPosition);
	
	// fix this? distance should be between closest explorer and minion instead of 
	// minion to next grid node
	//float distanceSquared = magnitudeSquared(direction);
	//if (distanceSquared < 1)
	//{
	//	return BehaviorStatus::Success;
	//}

	float dt = 0.001f * static_cast<float>(self.mTimer.GetDeltaTime());

	// speed per second
	vec2f targetVelocity = normalize(direction) * self.mBaseMoveSpeed * dt;

	// delta space for the current frame
	vec3f ds = targetVelocity;

	self.mPosition += ds;
	self.mIsTransformDirty = true;

	self.PlayStateAnimation(ANIM_STATE_RUN);
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

	float dt = 0.001f * static_cast<float>(self.mTimer.GetDeltaTime());

	// speed per second
	vec2f targetVelocity = normalize(vec2f(nextNode.worldPos) - vec2f(self.mPosition)) * self.mBaseMoveSpeed * dt;

	// delta space for the current frame
	vec3f ds = targetVelocity;

	// update position
	self.mPosition += ds;
	self.mIsTransformDirty = true;
	self.PlayStateAnimation(ANIM_STATE_RUN);

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

	vec2f dir = normalize(vec2f(self.mPosition) - vec2f(self.mLastPosition));
	self.LookAt(dir);

	return BehaviorStatus::Success;
}


BehaviorStatus MinionController::LookAtTarget(Behavior& bh, void* data)
{
	auto& self = *reinterpret_cast<MinionController*>(data);

	vec2f dir = normalize(vec2f(self.mTarget) - vec2f(self.mPosition));
	self.LookAt(dir);

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

	minion->mController->PlayStateAnimation(ANIM_STATE_IDLE);
}

void MinionController::OnMeleeHit(BaseSceneObject* minion, BaseSceneObject* other)
{
	if (other->Is<Explorer>())
	{
		Explorer* explorer = reinterpret_cast<Explorer*>(other);
		explorer->mHealth->TakeDamage(100.0f, false);
	}
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