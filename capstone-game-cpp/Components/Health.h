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

	void TakeDamage(float i, bool checkAuthority = true)
	{
		SetHealth(mCurrentHealth - i, checkAuthority);
	}

	void SetHealth(float val, bool checkAuthority = true)
	{
		// if server retransmit
		// else (client) send to server.
		mCurrentHealth = val;
		if (mCurrentHealth < 0) mCurrentHealth = 0;
		else if (mCurrentHealth > mMaxHealth) mCurrentHealth = mMaxHealth;

		OnHealthChange(val, checkAuthority);
	}	
	
	float GetHealth() { return mCurrentHealth; }
	float GetMaxHealth() { return mMaxHealth; }
	float GetHealthPerc() { return mCurrentHealth/mMaxHealth; }

	EXPOSE_CALLBACK_2(HealthChange, float, bool);
};