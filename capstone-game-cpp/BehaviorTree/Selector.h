#pragma once
#include "IterableComposite.h"

class Selector : public IterableComposite
{

public:
	Selector(BehaviorTree& bt, std::string name = "Selector");
};
