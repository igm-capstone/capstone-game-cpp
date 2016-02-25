#pragma once
#include "BehaviorTree/IterableComposite.h"

class PrioritySelector : public IterableComposite
{
public:
	PrioritySelector(BehaviorTree& bt, std::string name = "Priority Selector");
	static BehaviorStatus OnUpdate(Behavior& bh, void* data);
};
