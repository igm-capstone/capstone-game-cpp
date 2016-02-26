#pragma once
#include "IterableComposite.h"

class Sequence : public IterableComposite
{
public:
	Sequence(BehaviorTree& bt, std::string name = "Sequence");
};
