#include <stdafx.h>
#include "BaseSceneObject.h"
#include <Factory.h>
#include <ModelManager.h>


BaseSceneObject::BaseSceneObject()
	: mTransform(Factory<Transform>::Create()), mModel(nullptr)
{
}

BaseSceneObject::~BaseSceneObject()
{
	Factory<Transform>::Destroy(mTransform);
	if (mModel) mModel->Unlink(this);
}
