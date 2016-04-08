#pragma once
#include "BaseSceneObject.h"
#include <map>

class StatusEffect
{
	const int __pool_padding = 0xB0B0CACA;

	friend class Factory<StatusEffect>;
public:

	std::map<class Explorer*, float> mExplorers;
	std::map<class Minion*, float> mMinions;

	float mDuration;
	void* OnUpdate;

	static void OnPoisonUpdate(StatusEffect* self, BaseSceneObject* other, float seconds);
	static void OnSlowUpdate(StatusEffect* self, BaseSceneObject* other, float seconds);

private:

	StatusEffect();
	~StatusEffect();
};
