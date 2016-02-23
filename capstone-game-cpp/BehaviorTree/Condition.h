#pragma once
#include "Behavior.h"
#include <functional>

class Condition : public Behavior
{
public:
	BehaviorStatus Update() override;
	std::function<bool()> mCanRun;
};
