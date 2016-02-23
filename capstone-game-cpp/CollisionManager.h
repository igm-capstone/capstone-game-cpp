#pragma once
#include <Components/ColliderComponent.h>
#include "BVHTree.h"

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

	bool operator==(const Collision& other) const
	{
		return ((colliderA.BaseCollider == other.colliderA.BaseCollider && colliderB.BaseCollider == other.colliderB.BaseCollider) ||
			(colliderB.BaseCollider == other.colliderA.BaseCollider && colliderA.BaseCollider == other.colliderB.BaseCollider));
	}
};


class CollisionManager
{
public:
	BVHTree	mBVHTree;

	CollisionManager();
	~CollisionManager();

	void Initialize();

	void Update(double milliseconds);

	void DetectTriggers(std::vector<Collision>& frameCollisions);
	void DispatchTriggers(std::vector<Collision>& frameCollisions);
	inline void DispatchTriggerEnter(Collision* collision);
	inline void DispatchTriggerStay(Collision* collision);
	inline void DispatchTriggerExit(Collision* collision);

	void DetectCollisions();
	void ResolveCollisions();

private:
	std::vector<Collision>	mCollisions;
	std::vector<Collision>  mTriggers;
};

