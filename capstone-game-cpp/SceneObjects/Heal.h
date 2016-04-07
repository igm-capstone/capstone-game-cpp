#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Heal : public BaseSceneObject
{
	friend class Factory<Heal>;

public:
	class SphereColliderComponent*	mSphereColliderComponent;
	class NetworkID*				mNetworkID;

	float mHealthRestored;
	float mDuration;

	void Spawn(vec3f pos, int UUID, float duration);
	static void OnNetAuthorityChange(BaseSceneObject* obj, bool newAuth);
	static void OnNetSyncTransform(BaseSceneObject* obj, vec3f newPos, quatf newRot);
	static void OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other);

private:
	Heal();
	~Heal();
};
