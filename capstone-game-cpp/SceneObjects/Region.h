#pragma once
#include "SceneObjects/BaseSceneObject.h"
#include "Components/ColliderComponent.h"

class Region :
	public BaseSceneObject
{
	friend class Factory<Region>;

public:
	OrientedBoxColliderComponent*   mColliderComponent;
	PlaneCollider					mPlaneCollider;
	mat4f							mInvWorldMatrix;

	Region();
	~Region();
};

