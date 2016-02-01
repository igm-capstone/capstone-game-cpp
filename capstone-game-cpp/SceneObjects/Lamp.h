#include "SceneObjects/BaseSceneObject.h"
#include "Components/ColliderComponent.h"

class Lamp : public BaseSceneObject
{
	friend class Factory<Lamp>;

public:
	SphereColliderComponent*	mCollider;		// Collider for clicking. Note, this collider radius will differ from the actual light radius value.
	float mLightRadius;							// Radius of point light

private:
	Lamp() : 
		mCollider(Factory<SphereColliderComponent>::Create()),
		mLightRadius(0.0f)
	{
		mCollider->mSceneObject = this;
	}

	~Lamp()
	{
		mCollider = nullptr;
	}
};