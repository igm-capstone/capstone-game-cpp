#pragma once
#include "D:\Users\go4113\Capstone\capstone-game-cpp\Components\BaseComponent.h"
#include "Rig3D/Parametric.h"

class BoxColliderComponent :
	public BaseComponent
{
public:
	BoxCollider mCollider;

	BoxColliderComponent();
	~BoxColliderComponent();
};

class QuadColliderComponent :
	public BaseComponent
{
public:
	BoxCollider2D mCollider;

	QuadColliderComponent();
	~QuadColliderComponent();
};

class SphereColliderComponent : 
	public BaseComponent
{
public:
	SphereCollider mCollider;

	SphereColliderComponent();
	~SphereColliderComponent();
};