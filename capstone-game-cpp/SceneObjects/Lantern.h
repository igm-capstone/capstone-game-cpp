#pragma once
#include "SceneObjects\BaseSceneObject.h"
class Lantern :
	public BaseSceneObject
{
	friend class Factory<Lantern>;

	Lantern();
	~Lantern();
public:
	vec4f mLightColor;

	class SphereColliderComponent* mColliderComponent;
	class StatusEffect* mStatusEffect;
	class NetworkID* mNetworkID;

	float mDuration; 
	float mZOffset;
	bool mShouldDestroy;

	vec3f GetLightPosition()
	{
		vec3f position = mTransform->GetPosition();
		position.z += mZOffset;
		return position;
	}

	void Spawn(int UUID, vec3f position, float duration);
	void Update(float seconds);
	static void OnTriggerStay(BaseSceneObject* self, BaseSceneObject* other);
};

