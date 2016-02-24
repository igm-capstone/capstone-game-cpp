#pragma once
#include "Composite.h"

class IterableComposite : public Composite
{
public:
	IterableComposite(BehaviorTree& bt, BehaviorStatus breakStatus);

protected:
	class BehaviorTree* mBehaviorTree;
	std::vector<class Behavior*>::iterator mCurrent;
	BehaviorStatus mBreakStatus;

	static void OnChildComplete(void* data, BehaviorStatus status);
	static void OnInitialize(Behavior& self, void* data);
	static BehaviorStatus OnUpdate(Behavior& self, void* data);
};
