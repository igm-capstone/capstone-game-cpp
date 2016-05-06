#pragma once
#include "Factory.h"
#include <typeinfo>

using namespace Rig3D;

class BaseSceneObject
{
	int __pool_padding = 0xBABACACA;
public:
	size_t mClassID;
	const char* mClassName;

	union
	{
		Transform*		mTransform;
		mat4f*			mWorldMatrix;
	};

	// DO NOT set this variable manually. 
	// Use Application::SharedInstance().GetModelManager()->RequestModel("ModelName")->Link(this);
	// mModel will be populated.
	class ModelCluster*	mModel;
	bool mShouldDestroy;

	template <class T>
	bool Is() {
		return (mClassID == typeid(T).hash_code());
	}

protected:
	BaseSceneObject();
	~BaseSceneObject();
};
