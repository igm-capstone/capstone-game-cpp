#pragma once
#include "Behavior.h"
#include <functional>

class Condition : public Behavior
{
public:
	typedef bool(*ConditionCallback)(Behavior& self, void* data);

	void SetConditionCallback(ConditionCallback callback)
	{
		mOnCondition = callback;
	}
	
	Condition();


	static BehaviorStatus Update(Behavior& self, void* data);
	
protected:
	ConditionCallback mOnCondition;
};
