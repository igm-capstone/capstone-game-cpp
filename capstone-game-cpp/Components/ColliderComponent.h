#pragma once
#include "Rig3D/Parametric.h"
#include "BaseComponent.h"

struct Collision;

class BaseColliderComponent :
	public BaseComponent
{
public:
	bool mIsDynamic;

	EXPOSE_CALLBACK(CollisionEnter, Collision*)
	EXPOSE_CALLBACK(CollisionExit, Collision*)

protected:
	BaseColliderComponent() : mIsDynamic(true) {};
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