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


#define EXPOSE_CALLBACK(name, argtype) 	\
typedef void(*name##Callback)(BaseSceneObject*, argtype); \
private: \
	name##Callback m##name = nullptr; \
public: \
	void Register##name##Callback(name##Callback handler) { m##name = handler; } \
	void On##name(argtype arg) { if (m##name) return m##name(mSceneObject, arg); };
