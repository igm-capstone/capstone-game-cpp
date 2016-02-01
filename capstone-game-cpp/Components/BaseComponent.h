#pragma once
#include "SceneObjects\BaseSceneObject.h"

class BaseComponent {
	const int __pool_padding = 0xBABACACA;
public:
	BaseSceneObject*	mSceneObject;
	bool mIsActive;

protected:
	BaseComponent() : mSceneObject(nullptr), mIsActive(true) {}
	~BaseComponent() {};
};