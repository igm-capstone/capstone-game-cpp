#pragma once
#include "BaseComponent.h"
#include <BehaviorTree/Tree.h>
#include <Components/AnimationController.h>
#include <json.h>

class MinionController : public BaseComponent
{
protected:

	MinionController();
	~MinionController();
	BehaviorTree::Tree& CreateAttackSubtree();
	BehaviorTree::Tree& CreateWanderSubtree();
	BehaviorTree::Tree& CreateChaseSubtree();

public:
	LinearAllocator mAllocator;
	class AIManager& mAI;
	class Rig3D::Timer& mTimer;
	BehaviorTree::Tree* mBehaviorTree;
	quatf mBaseRotation;
	float mBaseMoveSpeed;
	float mAttackDamage;
	float mAttackRange;
	float mTurnRate;
	float mStunOnHitDuration;
	float mSplashRange;

	// variables used by BT
	vec3f mTarget;
	float mSpeed;
	float mThinkTime;
	float mWanderTime;
	vec3f mPosition;
	vec3f mLastPosition;
	float mAngle;
	int mDirectionIndex;
	bool mIsTransformDirty;
	static const vec2f sDirections[];

	bool Update(double milliseconds);
	bool IsExplorerInRange(int range, bool setTarget = true);
	bool LookAt(vec2f dir);
	quatf GetAdjustedRotation(float angle);

	// behavior tree code
	static bool IsExplorerInAttackRange(BehaviorTree::Behavior& bh, void* data);
	static bool IsExplorerInLockRange(BehaviorTree::Behavior& bh, void* data);
	static bool IsAttackInProgress(BehaviorTree::Behavior& bh, void* data);
	static bool IsExplorerVisible(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus StartAttack(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus TargetClosestExplorer(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveTowardsExplorer(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus Think(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveForward(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus UpdateWanderDirection(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus LookForward(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus LookAtTarget(BehaviorTree::Behavior& bh, void* data);

	void PlayStateAnimation(AnimationControllerState state);
	void PauseStateAnimation(AnimationControllerState state);

	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);
	static void OnMeleeHit(BaseSceneObject* minion, BaseSceneObject* other);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
};
