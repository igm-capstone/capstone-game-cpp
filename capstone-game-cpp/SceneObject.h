#pragma once
#include "Rig3D/Common/Transform.h"
#include "Rig3D/Graphics/Interface/IMesh.h"
#include "Rig3D/Parametric.h"

using namespace Rig3D;

class SceneObject
{
public:
	vec4f			mColor;
	IMesh*			mMesh;
	SceneObject*	mParent;

	union
	{
		Transform*		mTransform;
		mat4f*			mWorldMatrix;
	};

	union
	{
		BoxCollider*	mBoxCollider;
		SphereCollider*	mSphereCollider;
	};

	SceneObject();
	~SceneObject();
};

