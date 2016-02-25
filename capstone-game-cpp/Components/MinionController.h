#pragma once
#include "BaseComponent.h"

class MinionController : public BaseComponent
{
	friend class Factory<MinionController>;

	MinionController();
	~MinionController();

public:
	float mSpeed;
	class AIManager& mAI;
	class BehaviorTree* mBehaviorTree;

	bool Update(double milliseconds);

	// behavior tree code
	vec3f mTarget;
	static enum class BehaviorStatus FindTarget(class Behavior& bh, void* data);
	static enum class BehaviorStatus MoveTowardsTarget(class Behavior& bh, void* data);

	EXPOSE_CALLBACK_1(Move, vec3f)
};
