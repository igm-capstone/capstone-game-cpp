#pragma once
#include "Composite.h"

class Sequence : public Composite
{
public:
	Sequence(BehaviorTree& bt);

protected:
	class BehaviorTree* mBehaviorTree;

	void OnInitialize() override;
	void OnChildComplete(BehaviorStatus status);

	BehaviorStatus Update() override;

	std::vector<class Behavior*>::iterator mCurrent;

};
