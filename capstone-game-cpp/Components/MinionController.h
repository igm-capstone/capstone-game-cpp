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
	float mSpeed;
	float mThinkTime;
	class AIManager& mAI;
	class Rig3D::Timer& mTimer;
	BehaviorTree::Tree* mBehaviorTree;
	
	vec2f mDirection;
	vec2f mTarget;
	vec3f mPosition;
	float mAngle;
	bool mIsTransformDirty;

	static const vec2f sDirections[];

	bool Update(double milliseconds);
	quatf GetRotation(float angle);

	// behavior tree code
	static bool IsExplorerInRange(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveTowardsExplorer(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus Think(BehaviorTree::Behavior & bh, void * data);
	static BehaviorTree::BehaviorStatus FindTarget(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus RotateTowardsTarget(BehaviorTree::Behavior& bh, void* data);
	static BehaviorTree::BehaviorStatus MoveTowardsTarget(BehaviorTree::Behavior& bh, void* data);

	void PlayStateAnimation(AnimationControllerState state);
	void PauseStateAnimation(AnimationControllerState state);

	static void OnMeleeStart(void* obj);
	static void OnMeleeStop(void* obj);
	static void OnMeleeHit(BaseSceneObject* minion, BaseSceneObject* other);

	EXPOSE_CALLBACK_2(Move, vec3f, quatf)
};
