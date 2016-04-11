#pragma once
#include "BaseComponent.h"
#include <BehaviorTree/Tree.h>
#include <Components/AnimationController.h>

class MinionController : public BaseComponent
{
	friend class Factory<MinionController>;

	MinionController();
	~MinionController();

public:

	class AIManager& mAI;
	class Rig3D::Timer& mTimer;
	BehaviorTree::Tree* mBehaviorTree;
	
	// variables used by BT
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
	quatf GetAdjustedRotation(float angle);

	// behavior tree code
	static bool IsExplorerInRange(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveTowardsExplorer(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus Think(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus WanderTowardsDirection(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus UpdateWanderDirection(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus LookForward(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus UpdateTarget(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus RotateTowardsTarget(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveTowardsTarget(BehaviorTree::Behavior& bh, void* data);

	void PlayStateAnimation(AnimationControllerState state);
	void PauseStateAnimation(AnimationControllerState state);

	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);
	static void OnMeleeHit(BaseSceneObject* minion, BaseSceneObject* other);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
};
