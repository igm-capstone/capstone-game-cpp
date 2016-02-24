#pragma once
#include "Composite.h"

class Sequence : public Composite
{
public:
	Sequence(BehaviorTree& bt);

protected:
	class BehaviorTree* mBehaviorTree;
	std::vector<class Behavior*>::iterator mCurrent;

	static void OnChildComplete(void* data, BehaviorStatus status);
	static void OnInitialize(Behavior& self, void* data);
	static BehaviorStatus OnUpdate(Behavior& self, void* data);
};
