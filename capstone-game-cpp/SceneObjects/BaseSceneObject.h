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

private:
	friend class ModelCluster;
	class ModelCluster*	mModel;

public:
	ModelCluster* GetModelCluster() { return mModel; }

protected:
	BaseSceneObject();
	~BaseSceneObject();
};
