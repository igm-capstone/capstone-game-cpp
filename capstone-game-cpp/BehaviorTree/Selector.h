#pragma once
#include "Behavior.h"
#include "Composite.h"

class Selector : public Composite
{

public:
	Selector(BehaviorTree& bt);
	
protected:
	class BehaviorTree* mBehaviorTree;
	std::vector<class Behavior*>::iterator mCurrent;

	static void OnInitialize(Behavior& bh, void* data);
	static void OnChildComplete(void* observerData, BehaviorStatus status);
	static BehaviorStatus OnUpdate(Behavior& bh, void* data);
};
