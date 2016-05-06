#pragma once
#include "SceneObjects\BaseSceneObject.h"

class Trap : public BaseSceneObject
{
	friend class Factory<Trap>;

public:
	class SphereColliderComponent*	mSphereColliderComponent;
	class AnimationController*		mAnimationController;
	class NetworkID*				mNetworkID;
	class StatusEffect*				mEffect;

	float mDuration;				// Seconds
	float mEffectDuration;
	float mDelay;
	bool mShouldDestroy;

	void Spawn(int UUID, vec3f position, float duration);
	void SpawnPoison(int UUID, vec3f position, float duration);
	void SpawnSlow(int UUID, vec3f position, float duration);

	void Update(float seconds);
	static void OnTriggerEnter(BaseSceneObject* self, BaseSceneObject* other);
	static void OnTriggerStay(BaseSceneObject* self, BaseSceneObject* other);

	static void OnTrapOpenStart(void* obj);
	static void OnTrapOpenStop(void* obj);

private:
	Trap();
	~Trap();
};

