#pragma once
#include "Rig3D/Parametric.h"
#include "BaseComponent.h"

struct ColliderTraits
{
	bool isDynamic = true;
};

class BoxColliderComponent :
	public BaseComponent
{
public:
	ColliderTraits	mTraits;
	BoxCollider		mCollider;

	BoxColliderComponent();
	~BoxColliderComponent();
};

class OrientedBoxColliderComponent : 
	public BaseComponent
{
public:
	ColliderTraits		mTraits;
	OrientedBoxCollider mCollider;

	OrientedBoxColliderComponent();
	~OrientedBoxColliderComponent();
};

class QuadColliderComponent :
	public BaseComponent
{
public:
	ColliderTraits	mTraits;
	BoxCollider2D mCollider;

	QuadColliderComponent();
	~QuadColliderComponent();
};

class SphereColliderComponent : 
	public BaseComponent
{
public:
	ColliderTraits	mTraits;
	SphereCollider mCollider;

	SphereColliderComponent();
	~SphereColliderComponent();
};