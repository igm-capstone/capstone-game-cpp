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
		mCurrentHealth -= i;
		if (mCurrentHealth < 0) mCurrentHealth = 0;
		else if (mCurrentHealth > mMaxHealth) mCurrentHealth = mMaxHealth;
	}

	float GetHealth() { return mCurrentHealth; }
	float GetMaxHealth() { return mMaxHealth; }
	float GetHealthPerc() { return mCurrentHealth/mMaxHealth; }
};