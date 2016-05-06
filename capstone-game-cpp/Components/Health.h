#pragma once
#include "Components\BaseComponent.h"

class Health : public BaseComponent {
	friend class Factory<Health>;

	float	mCurrentHealth;
	float	mMaxHealth;
	float	mLastHitDirection;

	class NetworkID* mNetworkID;
	class NetworkManager& mNetworkManager;

	Health()
		: mNetworkManager(Singleton<NetworkManager>::SharedInstance())
	{
		
	}

	~Health()
	{
		
	}

	static void OnSyncHealth(BaseSceneObject* obj, float newVal, float newMax, float hitDirection);

public:

	void SetupNetworkID(NetworkID* networkID);
	
	void SetMaxHealth(float i);
	void TakeDamage(float i, float direction, bool checkAuthority = true);
	void SyncHealth(bool checkAuthority);
	void SetHealth(float val, bool checkAuthority = true);

	float GetHealth() { return mCurrentHealth; }
	float GetMaxHealth() { return mMaxHealth; }
	float GetHealthPerc() { return mCurrentHealth/mMaxHealth; }


	EXPOSE_CALLBACK_0(HealthToZero);
	EXPOSE_CALLBACK_0(HealthFromZero);
	EXPOSE_CALLBACK_3(HealthChange, float, float, float);
};