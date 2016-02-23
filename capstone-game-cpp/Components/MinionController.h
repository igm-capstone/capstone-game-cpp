#pragma once
#include "BaseComponent.h"
#include <BehaviorTree/BehaviorTree.h>

class MinionController : public BaseComponent
{
	friend class Factory<MinionController>;

	MinionController();
	~MinionController();

public:
	float mSpeed;
	BehaviorTree* mBehaviorTree;

	bool Update(double milliseconds);

	// behavior tree code
	vec3f mTarget;
	BehaviorStatus FindTarget();
	BehaviorStatus MoveTowardsTarget();

	EXPOSE_CALLBACK_1(Move, vec3f)
};
