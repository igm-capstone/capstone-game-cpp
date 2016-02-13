#pragma once
#include "Components\BaseComponent.h"

class Health : public BaseComponent {
private:
	float	mCurrentHealth;
	float	mMaxHealth;

public:

	void SetMaxHealth(float i)
	{
		mMaxHealth = i;
		mCurrentHealth = i;
	}

	void TakeDamage(float i)
	{
		SetHealth(mCurrentHealth - i);
	}

	void SetHealth(float val)
	{
		mCurrentHealth = val;
		if (mCurrentHealth < 0) mCurrentHealth = 0;
		else if (mCurrentHealth > mMaxHealth) mCurrentHealth = mMaxHealth;

		OnHealthChange(val);
	}	
	
	float GetHealth() { return mCurrentHealth; }
	float GetMaxHealth() { return mMaxHealth; }
	float GetHealthPerc() { return mCurrentHealth/mMaxHealth; }

	EXPOSE_CALLBACK_1(HealthChange, float);
};