#pragma once
#include "BaseSceneObject.h"
#include <map>

typedef void(*StatusUpdateCallback)(class StatusEffect* self, float seconds);

class StatusEffect
{
	const int __pool_padding = 0xB0B0CACA;

	friend class Factory<StatusEffect>;
public:

	std::map<class Explorer*, float> mExplorers;
	std::map<class Minion*, float> mMinions;

	float mDuration;
	StatusUpdateCallback mOnUpdateCallback;

	static void OnPoisonUpdate(StatusEffect* self, float seconds);
	static void OnSlowUpdate(StatusEffect* self, float seconds);

private:

	StatusEffect();
	~StatusEffect();
};
