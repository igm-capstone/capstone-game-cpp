#include "stdafx.h"
#include "Components/ColliderComponent.h"
#include "Rig3D/Intersection.h"

#pragma region BoxColliderComponent

BoxColliderComponent::BoxColliderComponent()
{
	mCollider.origin	= { 0.0f, 0.0f, 0.0f };
	mCollider.halfSize	= { 0.5f, 0.5f, 0.5f };
	mOnObbTest = IntersectAABBOBB;
}

BoxColliderComponent::~BoxColliderComponent()
{
}

int BoxColliderComponent::IntersectAABBOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	return Rig3D::IntersectOBBAABB(other->mCollider, reinterpret_cast<BoxColliderComponent*>(self)->mCollider);
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

	mOnObbTest = IntersectOBBOBB;
}

OrientedBoxColliderComponent::~OrientedBoxColliderComponent()
{
}

int OrientedBoxColliderComponent::IntersectOBBOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	return Rig3D::IntersectOBBOBB(reinterpret_cast<OrientedBoxColliderComponent*>(self)->mCollider, other->mCollider);
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

	mOnObbTest = IntersectSphereOBB;
}

SphereColliderComponent::~SphereColliderComponent()
{
}

int SphereColliderComponent::IntersectSphereOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	vec3f unused;
	return Rig3D::IntersectSphereOBB(reinterpret_cast<SphereColliderComponent*>(self)->mCollider, other->mCollider, unused);
}

#pragma endregion 

#pragma region PlaneColliderComponent

PlaneColliderComponent::PlaneColliderComponent()
{
	mCollider.normal = { 0.0f, 1.0f, 0.0 };
	mCollider.distance = 0.0f;
}
PlaneColliderComponent::~PlaneColliderComponent()
{
	
}

#pragma endregion 
