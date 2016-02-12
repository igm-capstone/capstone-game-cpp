#pragma once
#include "Components\BaseComponent.h"

class Health : public BaseComponent {
private:
	int		mCurrentHealth;
	int		mMaxHealth;

public:

	void SetMaxHealth(int i)
	{
		mMaxHealth = i;
		mCurrentHealth = 1;
	}

	void TakeDamage(int i)
	{
		mCurrentHealth -= i;
		if (mCurrentHealth < 0) mCurrentHealth = 0;
		else if (mCurrentHealth > mMaxHealth) mCurrentHealth = mMaxHealth;
	}

	int GetHealth() { return mCurrentHealth; }
};