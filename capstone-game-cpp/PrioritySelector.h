#pragma once
#include "BehaviorTree/IterableComposite.h"

class PrioritySelector : public IterableComposite
{
public:
	PrioritySelector(BehaviorTree& bt, std::string name = "Priority Selector");
protected:
	std::vector<class Behavior*>::iterator mLastChildComplete;

	static void OnInitialize(Behavior& bh, void* data);
	static void OnChildComplete(Behavior& bh, void* data, BehaviorStatus status);
	static BehaviorStatus OnUpdate(Behavior& bh, void* data);
};
