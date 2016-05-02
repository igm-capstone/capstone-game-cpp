#pragma once
#include "BaseSceneObject.h"
#include <map>

typedef void(*StatusEnterCallback)(class StatusEffect* self, class BaseSceneObject* other);
typedef void(*StatusUpdateCallback)(class StatusEffect* self, float seconds);
typedef void(*StatusDestroyCallback)(class StatusEffect* self);

class StatusEffect
{
	const int __pool_padding = 0xB0B0CACA;

	friend class Factory<StatusEffect>;
public:

	std::map<class Explorer*, float> mExplorers;
	std::map<class Minion*, float> mMinions;

	float	mDuration;
	bool	mIsActive;
	bool	mShouldDestroy;

	StatusEnterCallback	 mOnEnterCallback;
	StatusUpdateCallback mOnUpdateCallback;
	StatusDestroyCallback mOnDestroyCallback;

	void Update(float seconds);

	static void OnSlowEnter(StatusEffect* self, BaseSceneObject* other);

	static void OnPoisonUpdate(StatusEffect* self, float seconds);
	static void OnSlowUpdate(StatusEffect* self, float seconds);

	static void OnSlowDestroy(StatusEffect* self);
private:

	StatusEffect();
	~StatusEffect();
};
