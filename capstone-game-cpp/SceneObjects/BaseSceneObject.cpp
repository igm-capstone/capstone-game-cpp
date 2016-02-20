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
	if (mModel) mModel->Unlink(this);
}
