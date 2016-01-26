#pragma once
#include "SceneObjects\BaseSceneObject.h"

class BaseComponent {
public:
	bool				mIsActive;
	BaseSceneObject*	mSceneObject;

protected:
	BaseComponent() : mIsActive(true) {}
	~BaseComponent() {};
};