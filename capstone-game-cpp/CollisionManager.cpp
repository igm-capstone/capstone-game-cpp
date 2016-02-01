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
	for (uint32_t i = 0; i < mExplorers.size(); i++)
	{
		for (uint32_t j = i + 1; j < mExplorers.size(); j++)
		{
			if (IntersectSphereSphere(mExplorers[i]->mCollider->mCollider, mExplorers[j]->mCollider->mCollider))
			{
				pCollisions[collisionsCount].colliderA.SphereCollider = mExplorers[i]->mCollider;
				pCollisions[collisionsCount].colliderB.SphereCollider = mExplorers[j]->mCollider;

				float overlap = (mExplorers[i]->mCollider->mCollider.radius + mExplorers[j]->mCollider->mCollider.radius)
					- cliqCity::graphicsMath::magnitude(mExplorers[i]->mCollider->mCollider.origin - mExplorers[j]->mCollider->mCollider.origin);
				vec3f AtoB = (mExplorers[j]->mCollider->mCollider.origin - mExplorers[i]->mCollider->mCollider.origin);
				pCollisions[collisionsCount].minimumOverlap = AtoB * overlap;

				collisionsCount++;
			}
		}
	}

	// Minion / Minion Collisions

	// Explorer / Minion Collisions

	// Explorer / Wall Collisions

	for (uint32_t i = 0; i < mExplorers.size(); i++)
	{
		for (uint32_t j = 0; j < mWalls.size(); j++)
		{
			vec3f cp;
			if (IntersectSphereAABB(mExplorers[i]->mCollider->mCollider, mWalls[j]->mBoxCollider->mCollider, cp))
			{
				pCollisions[collisionsCount].colliderA.SphereCollider	= mExplorers[i]->mCollider;
				pCollisions[collisionsCount].colliderB.BoxCollider		= mWalls[j]->mBoxCollider;

				vec3f d = cp - mExplorers[i]->mCollider->mCollider.origin;
				vec3f r = cliqCity::graphicsMath::normalize(d) * mExplorers[i]->mCollider->mCollider.radius;
				pCollisions[collisionsCount].minimumOverlap = d - r;

				collisionsCount++;
			}
		}
	}

	mCollisionsCount = collisionsCount;
}

void CollisionManager::ResolveCollisions()
{
	Collision*	pCollisions = mCollisions;

	for (uint32_t i = 0; i < mCollisionsCount; i++)
	{
		// If object A and B are dynamic move both. If only A or B is dynamic only move A or B.
		if (pCollisions[i].colliderA.SphereCollider->mTraits.isDynamic && pCollisions[i].colliderB.SphereCollider->mTraits.isDynamic)
		{
			vec3f halfOverlap = pCollisions[i].minimumOverlap * 0.5f;
			vec3f posA = pCollisions[i].colliderA.SphereCollider->mSceneObject->mTransform->GetPosition() + halfOverlap;
			vec3f posB = pCollisions[i].colliderB.SphereCollider->mSceneObject->mTransform->GetPosition() - halfOverlap;

			pCollisions[i].colliderA.SphereCollider->mSceneObject->mTransform->SetPosition(posA);
			pCollisions[i].colliderB.SphereCollider->mSceneObject->mTransform->SetPosition(posB);

			pCollisions[i].colliderA.SphereCollider->mCollider.origin = posA;
			pCollisions[i].colliderB.SphereCollider->mCollider.origin = posB;
		}
		else if (pCollisions[i].colliderA.SphereCollider->mTraits.isDynamic && !pCollisions[i].colliderB.SphereCollider->mTraits.isDynamic)
		{
			vec3f posA = pCollisions[i].colliderA.SphereCollider->mSceneObject->mTransform->GetPosition() + pCollisions[i].minimumOverlap;
			pCollisions[i].colliderA.SphereCollider->mSceneObject->mTransform->SetPosition(posA);
			pCollisions[i].colliderA.SphereCollider->mCollider.origin = posA;

		}
		else if (!pCollisions[i].colliderA.SphereCollider->mTraits.isDynamic && pCollisions[i].colliderB.SphereCollider->mTraits.isDynamic)
		{
			vec3f posB = pCollisions[i].colliderB.SphereCollider->mSceneObject->mTransform->GetPosition() - pCollisions[i].minimumOverlap;
			pCollisions[i].colliderB.SphereCollider->mSceneObject->mTransform->SetPosition(posB);
			pCollisions[i].colliderB.SphereCollider->mCollider.origin = posB;
		}
	}

	// Last thing
	memset(mCollisions, 0, sizeof(Collision) * MAX_COLLISIONS);
}
