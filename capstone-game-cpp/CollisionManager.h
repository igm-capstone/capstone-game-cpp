#pragma once
#include <Components/ColliderComponent.h>
#include <Config.h>

struct Collision
{
	union Collider
	{
		BoxColliderComponent*		BoxCollider;
		SphereColliderComponent*	SphereCollider;
		QuadColliderComponent*		QuadCollider;
	} colliderA, colliderB;
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
	PoolAllocator mAllocator;
	Collision*	  mCollisions;
	uint32_t	  mCollisionCount;

	std::vector<Explorer*>  mExplorers;
	std::vector<Wall*>		mWalls;
};

