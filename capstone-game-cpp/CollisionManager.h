#pragma once
#include <Components/ColliderComponent.h>


struct Collision
{
	union Collider
	{
		BaseColliderComponent*			BaseCollider;
		BoxColliderComponent*			BoxCollider;
		OrientedBoxColliderComponent*	OBBCollider;
		SphereColliderComponent*		SphereCollider;
		QuadColliderComponent*			QuadCollider;
	} colliderA, colliderB;

	vec3f minimumOverlap;	// This is a vector in colliderA frame of reference
};

class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	void Initialize();
	void DetectCollisions();
	void ResolveCollisions();

private:
	Collision mCollisions[MAX_COLLISIONS];
	uint32_t  mCollisionsCount;
};

