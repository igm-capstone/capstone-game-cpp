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
	static BehaviorStatus FindTarget(Behavior& bh, void* data);
	static BehaviorStatus MoveTowardsTarget(Behavior& bh, void* data);

	EXPOSE_CALLBACK_1(Move, vec3f)
};
