#include "stdafx.h"
#include "CollisionManager.h"
#include "SceneObjects/Wall.h"
#include <Rig3D/Intersection.h>
#include "SceneObjects/Explorer.h"

#define NEW_COLLISION reinterpret_cast<Collision*>(mAllocator.Allocate())

CollisionManager::CollisionManager()
{
}


CollisionManager::~CollisionManager()
{
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

	for (int i = 0; i < mExplorers.size(); i++)
	{
		for (int j = i + 1; j < mExplorers.size(); j++)
		{
			if (IntersectAABBAABB(mExplorers[i]->mCollider->mCollider, mExplorers[j]->mCollider->mCollider))
			{
				Collision* collision = NEW_COLLISION;
				collision->colliderA.BoxCollider = mExplorers[i]->mCollider;
				collision->colliderB.BoxCollider = mExplorers[j]->mCollider;
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
				Collision* collision = NEW_COLLISION;
				collision->colliderA.BoxCollider = mExplorers[i]->mCollider;
				collision->colliderB.BoxCollider = mWalls[j]->mBoxCollider;
			}
		}
	}
}

void CollisionManager::ResolveCollisions()
{
	
}
