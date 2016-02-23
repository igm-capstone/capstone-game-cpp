#pragma once
#include <deque>
#include <BehaviorTree/Behavior.h>

class BehaviorTree
{
public:
	void Start(Behavior& bh, BehaviorObserver* observer = nullptr);
	void Stop(Behavior& bh, BehaviorStatus result);
	void Tick();
	bool Step();


protected:
	std::deque<Behavior*> mBehaviors;
};
