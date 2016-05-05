#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Explosion : public BaseSceneObject
{
	friend class Factory<Explosion>;

public:
	class SphereColliderComponent*	mSphereColliderComponent;
	class NetworkID*				mNetworkID;

	float mExplosionDamage;
	float mDuration;

	void Spawn(vec3f pos, int UUID, float duration);
	void Update(float seconds);
	static void OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other);

private:
	Explosion();
	~Explosion();
};

