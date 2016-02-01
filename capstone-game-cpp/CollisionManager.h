#pragma once
#include <Components/ColliderComponent.h>
#include <Config.h>

#define MAX_COLLISIONS 128

struct Collision
{
	union Collider
	{
		BoxColliderComponent*			BoxCollider;
		OrientedBoxColliderComponent*	OBBCollider;
		SphereColliderComponent*		SphereCollider;
		QuadColliderComponent*			QuadCollider;
	} colliderA, colliderB;

	vec3f minimumOverlap;	// This is a vector in colliderA frame of reference
};

class Explorer;
class Wall;

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

	std::vector<Explorer*>  mExplorers;
	std::vector<Wall*>		mWalls;
};

