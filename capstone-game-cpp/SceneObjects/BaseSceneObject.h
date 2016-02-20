#pragma once
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

	// DO NOT set this variable manually. 
	// Use Application::SharedInstance().GetModelManager()->RequestModel("ModelName")->Link(this);
	// mModel will be populated.
	class ModelCluster*	mModel;

protected:
	BaseSceneObject();
	~BaseSceneObject();
};
