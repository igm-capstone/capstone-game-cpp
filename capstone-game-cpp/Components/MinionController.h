#pragma once
#include "BaseComponent.h"

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
	class BehaviorTree* mBehaviorTree;

	bool Update(double milliseconds);
	// behavior tree code
	vec3f mTarget;
	static bool IsExplorerInRange(class Behavior& bh, void* data);
	static enum class BehaviorStatus MoveTowardsExplorer(class Behavior& bh, void* data);
	static enum class BehaviorStatus Think(class Behavior & bh, void * data);
	static enum class BehaviorStatus FindTarget(class Behavior& bh, void* data);
	static enum class BehaviorStatus MoveTowardsTarget(class Behavior& bh, void* data);

	EXPOSE_CALLBACK_1(Move, vec3f)
};
