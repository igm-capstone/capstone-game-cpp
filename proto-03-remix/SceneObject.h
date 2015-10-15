#pragma once
#include "Rig3D/Common/Transform.h"
#include "Rig3D/Graphics/Interface/IMesh.h"

using namespace Rig3D;

class SceneObject
{
public:
	Transform		mTransform;
	vec4f			mColor;
	IMesh*			mMesh;
	SceneObject*	mParent;

	SceneObject();
	~SceneObject();
};

