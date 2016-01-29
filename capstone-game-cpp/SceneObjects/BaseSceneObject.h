#pragma once
#include "Rig3D/Graphics/Interface/IMesh.h"
#include "Factory.h"

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
		mTransform = Factory<Transform>::Create();
	}
	~BaseSceneObject() {};
};