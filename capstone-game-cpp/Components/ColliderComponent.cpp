#include "stdafx.h"
#include "Components/ColliderComponent.h"

#pragma region BoxColliderComponent

BoxColliderComponent::BoxColliderComponent()
{
	mCollider.origin	= { 0.0f, 0.0f, 0.0f };
	mCollider.halfSize	= { 0.5f, 0.5f, 0.5f };
}

BoxColliderComponent::~BoxColliderComponent()
{
}

#pragma endregion 

#pragma region OrientedBoxColliderComponent

OrientedBoxColliderComponent::OrientedBoxColliderComponent()
{
	mCollider.axis[0] = { 1.0f, 0.0f, 0.0f };
	mCollider.axis[1] = { 0.0f, 1.0f, 0.0f };
	mCollider.axis[2] = { 0.0f, 0.0f, 1.0f };

	mCollider.origin = { 0.0f, 0.0f, 0.0f };
	mCollider.halfSize = { 0.5f, 0.5f, 0.5f };
}

OrientedBoxColliderComponent::~OrientedBoxColliderComponent()
{
}

#pragma endregion 

#pragma region QuadColliderComponent

QuadColliderComponent::QuadColliderComponent()
{
	mCollider.origin = { 0.0f, 0.0f };
	mCollider.halfSize = { 0.5f, 0.5f };
}

QuadColliderComponent::~QuadColliderComponent()
{
}

#pragma endregion 

#pragma region SphereColliderComponent

SphereColliderComponent::SphereColliderComponent()
{
	mCollider.origin = { 0.0f, 0.0f, 0.0f };
	mCollider.radius = 0.5f;
}

SphereColliderComponent::~SphereColliderComponent()
{
}

#pragma endregion 