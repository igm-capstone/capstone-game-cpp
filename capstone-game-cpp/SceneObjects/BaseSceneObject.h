#pragma once
#include "Rig3D/Graphics/Interface/IMesh.h"
#include "Rig3D/Parametric.h"
#include <Pool.h>

using namespace Rig3D;

class BaseSceneObject
{
	int __pool_padding = 0xBABACACA;
public:

	union
	{
		Transform*		mTransform;
		mat4f*			mWorldMatrix;
	};

protected:
	BaseSceneObject() : mTransform(nullptr)
	{
		mTransform = Pool<Transform>::Create();
	}
	~BaseSceneObject() {};
};