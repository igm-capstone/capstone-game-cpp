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
}

void CollisionManager::Initialize()
{
}

void CollisionManager::DetectCollisions()
{
	// Explorer / Explorer Collisions

	Collision*	pCollisions = mCollisions;
	uint32_t	collisionsCount = 0;
	for (Explorer& e1 : Factory<Explorer>())
	{
		for (Explorer& e2 : Factory<Explorer>())
		{
			if (&e1 == &e2) continue;

			if (IntersectSphereSphere(e1.mCollider->mCollider, e2.mCollider->mCollider))
			{
				pCollisions[collisionsCount].colliderA.SphereCollider = e1.mCollider;
				pCollisions[collisionsCount].colliderB.SphereCollider = e2.mCollider;

				float overlap = (e1.mCollider->mCollider.radius + e2.mCollider->mCollider.radius)
					- cliqCity::graphicsMath::magnitude(e1.mCollider->mCollider.origin - e2.mCollider->mCollider.origin);
				vec3f AtoB = (e2.mCollider->mCollider.origin - e1.mCollider->mCollider.origin);
				pCollisions[collisionsCount].minimumOverlap = AtoB * overlap;

				collisionsCount++;
			}
		}
	}

	// Minion / Minion Collisions

	// Explorer / Minion Collisions

	// Explorer / Wall Collisions

	for (Explorer& e : Factory<Explorer>())
	{
		for (Wall& w : Factory<Wall>())
		{
			vec3f cp;
			if (IntersectSphereOBB(e.mCollider->mCollider, w.mBoxCollider->mCollider, cp))
			{
				pCollisions[collisionsCount].colliderA.SphereCollider	= e.mCollider;
				pCollisions[collisionsCount].colliderB.OBBCollider		= w.mBoxCollider;

				vec3f d = cp - e.mCollider->mCollider.origin;
				vec3f r = cliqCity::graphicsMath::normalize(d) * e.mCollider->mCollider.radius;
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
