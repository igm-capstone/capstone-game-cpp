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
	BoxCollider		mCollider;
	ColliderTraits	mTraits;

	BoxColliderComponent();
	~BoxColliderComponent();
};

class QuadColliderComponent :
	public BaseComponent
{
public:
	BoxCollider2D mCollider;
	ColliderTraits	mTraits;

	QuadColliderComponent();
	~QuadColliderComponent();
};

class SphereColliderComponent : 
	public BaseComponent
{
public:
	SphereCollider mCollider;
	ColliderTraits	mTraits;

	SphereColliderComponent();
	~SphereColliderComponent();
};