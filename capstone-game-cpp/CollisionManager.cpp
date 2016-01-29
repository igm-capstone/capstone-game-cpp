#include "stdafx.h"
#include "CollisionManager.h"
#include "SceneObjects/Wall.h"
#include <Rig3D/Intersection.h>
#include "SceneObjects/Explorer.h"

CollisionManager::CollisionManager() : mCollisionsCount(0)
{
}


CollisionManager::~CollisionManager()
{
	mExplorers.clear();
	mWalls.clear();
}

void CollisionManager::Initialize()
{
	for (Explorer& e : Factory<Explorer>())
	{
		mExplorers.push_back(&e);
	}

	for (Wall& w : Factory<Wall>())
	{
		mWalls.push_back(&w);
	}
}

void CollisionManager::DetectCollisions()
{
	// Explorer / Explorer Collisions

	Collision*	pCollisions = mCollisions;
	uint32_t	collisionsCount = 0;
	for (int i = 0; i < mExplorers.size(); i++)
	{
		for (int j = i + 1; j < mExplorers.size(); j++)
		{
			if (IntersectAABBAABB(mExplorers[i]->mCollider->mCollider, mExplorers[j]->mCollider->mCollider))
			{
				pCollisions[collisionsCount].colliderA.BoxCollider = mExplorers[i]->mCollider;
				pCollisions[collisionsCount].colliderB.BoxCollider = mExplorers[j]->mCollider;
				collisionsCount++;
			}
		}
	}

	// Minion / Minion Collisions

	// Explorer / Minion Collisions

	// Explorer / Wall Collisions

	for (int i = 0; i < mExplorers.size(); i++)
	{
		for (int j = 0; j < mWalls.size(); j++)
		{
			if (IntersectAABBAABB(mExplorers[i]->mCollider->mCollider, mWalls[j]->mBoxCollider->mCollider))
			{
				pCollisions[collisionsCount].colliderA.BoxCollider = mExplorers[i]->mCollider;
				pCollisions[collisionsCount].colliderB.BoxCollider = mWalls[j]->mBoxCollider;
				collisionsCount++;
			}
		}
	}

	mCollisionsCount = collisionsCount;
}

void CollisionManager::ResolveCollisions()
{
	for (int i = 0; i < mCollisionsCount; i++)
	{
		// Push out via MTV
	}

	// Last thing
	memset(mCollisions, 0, sizeof(Collision) * MAX_COLLISIONS);
}
