#pragma once
#include "SceneObjects\BaseSceneObject.h"
#include "Components/ColliderComponent.h"

class LegacySceneObject : public BaseSceneObject
{
public:
	vec4f			mColor;
	IMesh*			mMesh;

	union {
		BoxColliderComponent*		mBoxCollider;
		SphereColliderComponent*	mSphereCollider;
	};

	LegacySceneObject() : mMesh(nullptr), mBoxCollider(nullptr) {}
	~LegacySceneObject() {};
};