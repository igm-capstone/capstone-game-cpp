#include "stdafx.h"
#include "Components/ColliderComponent.h"
#include "Rig3D/Intersection.h"

#pragma region BoxColliderComponent

BoxColliderComponent::BoxColliderComponent()
{
	mCollider.origin	= { 0.0f, 0.0f, 0.0f };
	mCollider.halfSize	= { 0.5f, 0.5f, 0.5f };
	mOnObbTest = IntersectAABBOBB;
	mOnSphereTest = IntersectAABBSphere;
	mOnRayTest = IntersectRayAABB;
}

BoxColliderComponent::~BoxColliderComponent()
{
}

int BoxColliderComponent::IntersectAABBOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	return Rig3D::IntersectOBBAABB(other->mCollider, reinterpret_cast<BoxColliderComponent*>(self)->mCollider);
}

int BoxColliderComponent::IntersectAABBSphere(BaseColliderComponent* self, SphereColliderComponent* other)
{
	vec3f unused;
	return Rig3D::IntersectSphereAABB(other->mCollider, reinterpret_cast<BoxColliderComponent*>(self)->mCollider, unused);
}

int BoxColliderComponent::IntersectRayAABB(Ray<vec3f> ray, BaseColliderComponent* self, vec3f& poi, float& t)
{
	return Rig3D::IntersectRayAABB(ray, reinterpret_cast<BoxColliderComponent*>(self)->mCollider, poi, t);
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
	mOnSphereTest = IntersectOBBSphere;
	mOnRayTest = IntersectRayOBB;
}

OrientedBoxColliderComponent::~OrientedBoxColliderComponent()
{
}

int OrientedBoxColliderComponent::IntersectOBBOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	return Rig3D::IntersectOBBOBB(reinterpret_cast<OrientedBoxColliderComponent*>(self)->mCollider, other->mCollider);
}

int OrientedBoxColliderComponent::IntersectOBBSphere(BaseColliderComponent* self, SphereColliderComponent* other)
{
	vec3f unused;
	return Rig3D::IntersectSphereOBB(other->mCollider, reinterpret_cast<OrientedBoxColliderComponent*>(self)->mCollider, unused);
}

int OrientedBoxColliderComponent::IntersectRayOBB(Ray<vec3f> ray, BaseColliderComponent* self, vec3f& poi, float& t)
{
	return Rig3D::IntersectRayOBB(ray, reinterpret_cast<OrientedBoxColliderComponent*>(self)->mCollider, poi, t);
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

	mOnObbTest		= IntersectSphereOBB;
	mOnSphereTest	= IntersectSphereSphere;
	mOnRayTest = IntersectRaySphere;
}

SphereColliderComponent::~SphereColliderComponent()
{
}

int SphereColliderComponent::IntersectSphereOBB(class BaseColliderComponent* self, class OrientedBoxColliderComponent* other)
{
	vec3f unused;
	return Rig3D::IntersectSphereOBB(reinterpret_cast<SphereColliderComponent*>(self)->mCollider, other->mCollider, unused);
}

int SphereColliderComponent::IntersectSphereSphere(BaseColliderComponent* self, SphereColliderComponent* other)
{
	return Rig3D::IntersectSphereSphere(other->mCollider, reinterpret_cast<SphereColliderComponent*>(self)->mCollider);
}

int SphereColliderComponent::IntersectRaySphere(Ray<vec3f> ray, BaseColliderComponent* self, vec3f& poi, float& t)
{
	return Rig3D::IntersectRaySphere(ray, reinterpret_cast<SphereColliderComponent*>(self)->mCollider, poi, t);
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
