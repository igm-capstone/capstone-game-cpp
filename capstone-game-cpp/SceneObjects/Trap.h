#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Trap : public BaseSceneObject
{
	friend class Factory<Trap>;

public:
	class SphereColliderComponent*	mSphereColliderComponent;
	class NetworkID*				mNetworkID;
	class StatusEffect*				mEffect;

	float mDuration;
	float mEffectDuration;

	void Spawn(int UUID, vec3f position, float duration);

	static void OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other);

private:
	Trap();
	~Trap();
};

