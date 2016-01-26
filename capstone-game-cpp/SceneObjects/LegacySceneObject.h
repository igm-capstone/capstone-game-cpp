#pragma once
#include "SceneObjects\BaseSceneObject.h"

class LegacySceneObject : public BaseSceneObject
{
public:
	vec4f			mColor;
	IMesh*			mMesh;

	union {
		BoxCollider*	mBoxCollider;
		SphereCollider* mSphereCollider;
	};

	LegacySceneObject() : mMesh(nullptr), mBoxCollider(nullptr) {}
	~LegacySceneObject() {};
};