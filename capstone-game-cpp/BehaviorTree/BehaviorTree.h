﻿#pragma once
#include <deque>
#include <BehaviorTree/Behavior.h>

class BehaviorTree
{
public:
	void Start(Behavior& bh);
	void Start(Behavior& bh, ObserverCallback observer, void* observerData = nullptr);
	void Stop(Behavior& bh, BehaviorStatus result);
	void Tick(void* userData = nullptr);
	bool Step(void* userData);

protected:
	std::deque<Behavior*> mBehaviors;
};
