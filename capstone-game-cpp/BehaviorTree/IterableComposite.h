#pragma once
#include "Composite.h"

class IterableComposite : public Composite
{
public:
	IterableComposite(BehaviorTree& bt, BehaviorStatus breakStatus, std::string name = "Iterable Composite");

protected:
	class BehaviorTree* mBehaviorTree;
	std::vector<class Behavior*>::iterator mCurrent;
	BehaviorStatus mBreakStatus;

	static void OnChildComplete(Behavior& self, void* data, BehaviorStatus status);
	static void OnInitialize(Behavior& self, void* data);
	static void OnFlush(Behavior& bh, void* data);
	static BehaviorStatus OnUpdate(Behavior& self, void* data);
};
