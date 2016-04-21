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
		float prevHealth = mCurrentHealth;
		mCurrentHealth = val;

		if (mCurrentHealth < 0)
		{
			mCurrentHealth = 0;
		}
		else if (mCurrentHealth > mMaxHealth)
		{
			mCurrentHealth = mMaxHealth;
		}

		if (prevHealth > 0.0f && mCurrentHealth == 0.0f)
		{
			OnHealthToZero();
		}
		else if (prevHealth == 0.0f && mCurrentHealth > 0.0f)
		{
			OnHealthFromZero();
		}

		OnHealthChange(val, checkAuthority);
	}	
	
	float GetHealth() { return mCurrentHealth; }
	float GetMaxHealth() { return mMaxHealth; }
	float GetHealthPerc() { return mCurrentHealth/mMaxHealth; }


	EXPOSE_CALLBACK_0(HealthToZero);
	EXPOSE_CALLBACK_0(HealthFromZero);
	EXPOSE_CALLBACK_2(HealthChange, float, bool);
};