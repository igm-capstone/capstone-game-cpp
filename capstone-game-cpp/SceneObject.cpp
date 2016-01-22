#include "stdafx.h"
#include "SceneObject.h"



SceneObject::SceneObject() : mTransform(nullptr), mMesh(nullptr), mParent(nullptr), mIsActive(true), mHasAuthority(true)
{
}


SceneObject::~SceneObject()
{
}
