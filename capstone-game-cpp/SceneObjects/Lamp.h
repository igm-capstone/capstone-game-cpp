#include "SceneObjects/BaseSceneObject.h"
#include "Components/ColliderComponent.h"

class Lamp : public BaseSceneObject
{
	friend class Factory<Lamp>;

public:
	SphereColliderComponent*	mCollider;		// Collider for clicking. Note, this collider radius will differ from the actual light radius value.
	IMesh*						mConeMesh;
	vec4f mLightColor;
	vec3f mLightDirection;
	float mLightRadius;							// Linear range of spot light
	float mLightAngle;
private:
	Lamp() : 
		mCollider(Factory<SphereColliderComponent>::Create()),
		mConeMesh(nullptr),
		mLightRadius(0.0f),
		mLightAngle(0.0f)
	{
		mCollider->mSceneObject = this;
	}

	~Lamp()
	{
		mCollider = nullptr;
	}
};