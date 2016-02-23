#pragma once
#include "Rig3D/Parametric.h"
#include "BaseComponent.h"

enum CLayer : short
{
	COLLISION_LAYER_ROOT = -2,
	COLLISION_LAYER_QUADRANT,
	COLLISION_LAYER_FLOOR,
	COLLISION_LAYER_WALL,
	COLLISION_LAYER_EXPLORER,
	COLLISION_LAYER_MINION,
	COLLISION_LAYER_SKILL,
	COLLISION_LAYER_LAMP
};

class BaseColliderComponent :
	public BaseComponent
{
public:
	CLayer	mLayer;
	bool	mIsDynamic;
	bool	mIsTrigger;

	EXPOSE_CALLBACK_2(CollisionEnter, BaseSceneObject*, vec3f)
	EXPOSE_CALLBACK_1(CollisionExit, BaseSceneObject*)

	EXPOSE_CALLBACK_1(TriggerEnter, BaseSceneObject*)
	EXPOSE_CALLBACK_1(TriggerStay, BaseSceneObject*)
	EXPOSE_CALLBACK_1(TriggerExit, BaseSceneObject*)

protected:
	BaseColliderComponent() : mLayer(COLLISION_LAYER_ROOT), mIsDynamic(true), mIsTrigger(false) {};
	~BaseColliderComponent() {};
};

class BoxColliderComponent :
	public BaseColliderComponent
{
public:
	BoxCollider		mCollider;

	BoxColliderComponent();
	~BoxColliderComponent();
};

class OrientedBoxColliderComponent : 
	public BaseColliderComponent
{
public:
	OrientedBoxCollider mCollider;

	OrientedBoxColliderComponent();
	~OrientedBoxColliderComponent();
};

class QuadColliderComponent :
	public BaseColliderComponent
{
public:
	BoxCollider2D mCollider;

	QuadColliderComponent();
	~QuadColliderComponent();
};

class SphereColliderComponent : 
	public BaseColliderComponent
{
public:
	SphereCollider mCollider;

	SphereColliderComponent();
	~SphereColliderComponent();
};

class PlaneColliderComponent :
	public BaseColliderComponent
{
public:
	PlaneCollider mCollider;

	PlaneColliderComponent();
	~PlaneColliderComponent();
};