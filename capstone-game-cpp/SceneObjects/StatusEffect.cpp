#include "stdafx.h"

#include "StatusEffect.h"
#include "Explorer.h"
#include "Minion.h"
#include "Components/Health.h"

StatusEffect::StatusEffect() : mDuration(0.25f), mIsActive(false), mShouldDestroy(false), mOnUpdateCallback(nullptr)
{
	
}

StatusEffect::~StatusEffect()
{
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

void StatusEffect::OnPoisonUpdate(StatusEffect* self, float seconds)
{
	for (std::map<Explorer*, float>::iterator iter = self->mExplorers.begin(); iter != self->mExplorers.end(); ++iter)
	{
		if (iter->second > 0)
		{
			iter->first->mHealth->SetHealth(iter->first->mHealth->GetHealth() - 25.0f);
			iter->second -= seconds;
		}
	}

	for (std::map<Minion*, float>::iterator iter = self->mMinions.begin(); iter != self->mMinions.end(); ++iter)
	{
		if (iter->second > 0)
		{
			iter->first->mHealth->SetHealth(iter->first->mHealth->GetHealth() - 25.0f);
			iter->second -= seconds;			
		}
	}
}

void StatusEffect::OnSlowUpdate(StatusEffect* self, float seconds)
{
	// How do minion handle slow down?
}