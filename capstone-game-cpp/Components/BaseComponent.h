#pragma once
#include "SceneObjects\BaseSceneObject.h"

class BaseComponent {
	const int __pool_padding = 0xBABACACA;
public:
	bool mIsActive;
	BaseSceneObject*	mSceneObject;

protected:
	BaseComponent() : mIsActive(true) {}
	~BaseComponent() {};
};