#pragma once
#include "SceneObjects\BaseSceneObject.h"

class BaseComponent {
	const int __pool_padding = 0xB0B0CACA;

public:
	BaseSceneObject*	mSceneObject;
	bool mIsActive;

protected:
	BaseComponent() : mSceneObject(nullptr), mIsActive(true) {}
	~BaseComponent() {};
};


#define EXPOSE_CALLBACK_0(name) 	\
public: \
	typedef void(*name##Callback)(BaseSceneObject*); \
	void Register##name##Callback(name##Callback handler) { m##name = handler; } \
	void On##name() { if (m##name) return m##name(mSceneObject); };\
private: \
	name##Callback m##name = nullptr;

#define EXPOSE_CALLBACK_1(name, argtype) 	\
public: \
	typedef void(*name##Callback)(BaseSceneObject*, argtype); \
	void Register##name##Callback(name##Callback handler) { m##name = handler; } \
	void On##name(argtype arg) { if (m##name) return m##name(mSceneObject, arg); };\
private: \
	name##Callback m##name = nullptr;

#define EXPOSE_CALLBACK_2(name, argtype1, argtype2) 	\
public: \
	typedef void(*name##Callback)(BaseSceneObject*, argtype1, argtype2); \
	void Register##name##Callback(name##Callback handler) { m##name = handler; } \
	void On##name(argtype1 arg1, argtype2 arg2) { if (m##name) return m##name(mSceneObject, arg1, arg2); };\
private: \
	name##Callback m##name = nullptr;


#define EXPOSE_CALLBACK_BOOL_3(name, argtype1, argtype2, argtype3) 	\
public: \
	typedef bool(*name##Callback)(BaseSceneObject*, argtype1, argtype2, argtype3); \
	void Register##name##Callback(name##Callback handler) { m##name = handler; } \
	bool On##name(argtype1 arg1, argtype2 arg2, argtype3 arg3) { if (m##name) return m##name(mSceneObject, arg1, arg2, arg3); return false; };\
private: \
	name##Callback m##name = nullptr;
