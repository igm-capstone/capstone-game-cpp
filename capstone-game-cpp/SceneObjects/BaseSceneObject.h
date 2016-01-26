#pragma once
#include "Rig3D/Common/Transform.h"
#include "Rig3D/Graphics/Interface/IMesh.h"
#include "Rig3D/Parametric.h"

using namespace Rig3D;

class BaseSceneObject
{
public:
	union
	{
		Transform*		mTransform;
		mat4f*			mWorldMatrix;
	};

protected:
	BaseSceneObject() : mTransform(nullptr) {}
	~BaseSceneObject() {};
};