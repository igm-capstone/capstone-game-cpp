#include "stdafx.h"

#include "StatusEffect.h"
#include "Explorer.h"
#include "Minion.h"
#include "Components/Health.h"
#include "Components/ExplorerController.h"
#include "Components/MinionController.h"

StatusEffect::StatusEffect() : 
	mDuration(0.25f), 
	mIsActive(false), 
	mShouldDestroy(false), 
	mOnEnterCallback(nullptr),
	mOnUpdateCallback(nullptr),
	mOnDestroyCallback(nullptr)
{
	
}

StatusEffect::~StatusEffect()
{
	if (mOnDestroyCallback)
	{
		mOnDestroyCallback(this);
	}

	mExplorers.clear();
	mMinions.clear();
}

void StatusEffect::Update(float seconds)
{
	mOnUpdateCallback(this, seconds);
	mDuration -= seconds;
	if (mDuration <= 0.0f)
	{
		mShouldDestroy = true;
	}
}

void StatusEffect::OnSlowEnter(StatusEffect* self, BaseSceneObject* other)
{
	if (other->Is<Minion>())
	{
		Minion* pMinion = reinterpret_cast<Minion*>(other);
		pMinion->mController->mSpeedMultiplier = 0.25f;
	}
}

void StatusEffect::OnPoisonUpdate(StatusEffect* self, float seconds)
{
	for (std::map<Explorer*, float>::iterator iter = self->mExplorers.begin(); iter != self->mExplorers.end(); ++iter)
	{
		if (iter->second > 0.0f)
		{
			iter->first->mHealth->TakeDamage(5.0f * seconds, FLT_MIN);
			iter->second -= seconds;
		}
	}

	for (std::map<Minion*, float>::iterator iter = self->mMinions.begin(); iter != self->mMinions.end(); ++iter)
	{
		if (iter->second > 0.0f)
		{
			iter->first->mHealth->TakeDamage(5.0f * seconds, FLT_MIN);
			iter->second -= seconds;
		}
	}
}

void StatusEffect::OnSlowUpdate(StatusEffect* self, float seconds)
{
	for (std::map<Explorer*, float>::iterator iter = self->mExplorers.begin(); iter != self->mExplorers.end(); ++iter)
	{
		if (iter->second > 0.0f)
		{
			iter->second -= seconds;
		}
		else
		{
			iter->first->mController->mSpeedMultiplier = 1.0f;
		}
	}

	for (std::map<Minion*, float>::iterator iter = self->mMinions.begin(); iter != self->mMinions.end(); ++iter)
	{
		if (iter->second > 0.0f)
		{
			iter->second -= seconds;
		}
		else
		{
			iter->first->mController->mSpeedMultiplier = 1.0f;
		}
	}
}

void StatusEffect::OnSlowDestroy(StatusEffect* self)
{
	for (std::map<Explorer*, float>::iterator iter = self->mExplorers.begin(); iter != self->mExplorers.end(); ++iter)
	{
		iter->first->mController->mSpeedMultiplier = 1.0f;
	}

	for (std::map<Minion*, float>::iterator iter = self->mMinions.begin(); iter != self->mMinions.end(); ++iter)
	{
		iter->first->mController->mSpeedMultiplier = 1.0f;
	}
}