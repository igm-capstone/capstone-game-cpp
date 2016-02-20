#include "stdafx.h"
#include "CollisionManager.h"
#include <Rig3D/Intersection.h>
#include "SceneObjects/Explorer.h"
#include "SceneObjects/DominationPoint.h"
#include "SceneObjects/StaticCollider.h"
#include <algorithm>

// Functor used to find matching collisions
namespace
{
	struct CollisionFinder
	{
		Collision* collision;

		CollisionFinder(Collision* pCollision) : collision(pCollision)
		{
			
		}

		bool operator()(const Collision& other) const
		{
			return ((collision->colliderA.BaseCollider == other.colliderA.BaseCollider && collision->colliderB.BaseCollider == other.colliderB.BaseCollider) ||
				(collision->colliderB.BaseCollider == other.colliderA.BaseCollider && collision->colliderA.BaseCollider == other.colliderB.BaseCollider));
		}
	};
}

CollisionManager::CollisionManager()
{
}

CollisionManager::~CollisionManager()
{
}

void CollisionManager::Initialize()
{
	mCollisions.reserve(MAX_COLLISIONS);
	mTriggers.reserve(MAX_COLLISIONS);

	mBVHTree.Initialize();
}

void CollisionManager::Update(double milliseconds)
{
	mBVHTree.Update();

	std::vector<Collision> frameCollisions;

	DetectTriggers(frameCollisions);
	DispatchTriggers(frameCollisions);

	frameCollisions.clear();

	DetectCollisions();
	ResolveCollisions();
}

void CollisionManager::DetectTriggers(std::vector<Collision>& frameCollisions)
{
	// Explorer / Domination Point
	
	// The most domination point triggers we can have is 4. An explorer can only be overlapping one domination point at a time.
	frameCollisions.reserve(MAX_EXPLORERS);

	for (Explorer& e : Factory<Explorer>())
	{
		for (DominationPoint& d : Factory<DominationPoint>())
		{
			if (IntersectSphereSphere(e.mCollider->mCollider, d.mCollider->mCollider))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.SphereCollider = e.mCollider;
				pTrigger->colliderB.SphereCollider = d.mCollider;
			}
		}
	}
}

void CollisionManager::DispatchTriggers(std::vector<Collision>& frameCollisions)
{
	std::vector<Collision>& triggers	= mTriggers;
	uint32_t triggerCount				= mTriggers.size();

	std::vector<uint32_t> indicesToRemove;
	indicesToRemove.reserve(triggerCount);

	// Look for frame triggers matching our cached triggers

	for (uint32_t i = 0; i < triggerCount; i++)
	{
		// Search frame triggers list for matching cached triggers
		std::vector<Collision>::iterator match = std::find_if(frameCollisions.begin(), frameCollisions.end(), CollisionFinder(&triggers[i]));

		if (match != frameCollisions.end())
		{
			// Trigger exists. Fire stay event and remove from list.
			DispatchTriggerStay(&triggers[i]);
			frameCollisions.erase(match);
		}
		else
		{
			// Trigger no longer exists and should be removed
			indicesToRemove.push_back(i);
		}
	}

	// Remove invalid triggers

	while (indicesToRemove.size())
	{
		DispatchTriggerExit(&triggers[indicesToRemove.back()]);
		triggers.erase(triggers.begin() + indicesToRemove.back());
		indicesToRemove.pop_back();
	}

	// Add new triggers

	for (uint32_t i = 0; i < frameCollisions.size(); i++)
	{
		DispatchTriggerEnter(&frameCollisions[i]);
		triggers.push_back(frameCollisions[i]);
	}
}

void CollisionManager::DispatchTriggerEnter(Collision* collision)
{
	if (collision->colliderA.BaseCollider->mIsTrigger)
	{
		collision->colliderA.BaseCollider->OnTriggerEnter(collision->colliderB.BaseCollider->mSceneObject);
	}

	if (collision->colliderB.BaseCollider->mIsTrigger)
	{
		collision->colliderB.BaseCollider->OnTriggerEnter(collision->colliderA.BaseCollider->mSceneObject);
	}
}

void CollisionManager::DispatchTriggerStay(Collision* collision)
{
	if (collision->colliderA.BaseCollider->mIsTrigger)
	{
		collision->colliderA.BaseCollider->OnTriggerStay(collision->colliderB.BaseCollider->mSceneObject);
	}

	if (collision->colliderB.BaseCollider->mIsTrigger)
	{
		collision->colliderB.BaseCollider->OnTriggerStay(collision->colliderA.BaseCollider->mSceneObject);
	}
}

void CollisionManager::DispatchTriggerExit(Collision* collision)
{
	if (collision->colliderA.BaseCollider->mIsTrigger)
	{
		collision->colliderA.BaseCollider->OnTriggerExit(collision->colliderB.BaseCollider->mSceneObject);
	}

	if (collision->colliderB.BaseCollider->mIsTrigger)
	{
		collision->colliderB.BaseCollider->OnTriggerExit(collision->colliderA.BaseCollider->mSceneObject);
	}
}

void CollisionManager::DetectCollisions()
{
	// Explorer / Explorer Collisions

	std::vector<Collision>& collisions = mCollisions;

	for (Explorer& e1 : Factory<Explorer>())
	{
		for (Explorer& e2 : Factory<Explorer>())
		{
			if (&e1 == &e2) continue;

			if (IntersectSphereSphere(e1.mCollider->mCollider, e2.mCollider->mCollider))
			{
				float overlap = (e1.mCollider->mCollider.radius + e2.mCollider->mCollider.radius)
					- magnitude(e1.mCollider->mCollider.origin - e2.mCollider->mCollider.origin);
				vec3f AtoB = (e2.mCollider->mCollider.origin - e1.mCollider->mCollider.origin);

				collisions.push_back(Collision());

				Collision* pCollision = &collisions.back();
				pCollision->colliderA.SphereCollider = e1.mCollider;
				pCollision->colliderB.SphereCollider = e2.mCollider;
				pCollision->minimumOverlap = - AtoB * overlap;

				e1.mCollider->OnCollisionEnter(&e2, overlap);
			}
		}
	}

	// Minion / Minion Collisions

	// Explorer / Minion Collisions

	// Explorer / Wall Collisions

	for (Explorer& e : Factory<Explorer>())
	{
		for (StaticCollider& w : Factory<StaticCollider>())
		{
			vec3f cp;
			
			if (IntersectSphereOBB(e.mCollider->mCollider, w.mBoxCollider->mCollider, cp))
			{
				vec3f d = cp - e.mCollider->mCollider.origin;
				vec3f r = normalize(d) * e.mCollider->mCollider.radius;
				collisions.push_back(Collision());

				Collision* pCollision = &collisions.back();
				pCollision->colliderA.SphereCollider	= e.mCollider;
				pCollision->colliderB.OBBCollider		= w.mBoxCollider;
				pCollision->minimumOverlap				= d - r;

				e.mCollider->OnCollisionEnter(&w, d - r);
			}
		}
	}
}

void CollisionManager::ResolveCollisions()
{
	std::vector<Collision>& collisions = mCollisions;

	for (uint32_t i = 0; i < collisions.size(); i++)
	{
		// If object A and B are dynamic move both. If only A or B is dynamic only move A or B.
		if (collisions[i].colliderA.SphereCollider->mIsDynamic && collisions[i].colliderB.SphereCollider->mIsDynamic)
		{
			vec3f halfOverlap = collisions[i].minimumOverlap * 0.5f;
			vec3f posA = collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->GetPosition() + halfOverlap;
			vec3f posB = collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->GetPosition() - halfOverlap;

			collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->SetPosition(posA);
			collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->SetPosition(posB);

			collisions[i].colliderA.SphereCollider->mCollider.origin = posA;
			collisions[i].colliderB.SphereCollider->mCollider.origin = posB;
		}
		else if (collisions[i].colliderA.SphereCollider->mIsDynamic && !collisions[i].colliderB.SphereCollider->mIsDynamic)
		{
			vec3f posA = collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->GetPosition() + collisions[i].minimumOverlap;
			collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->SetPosition(posA);
			collisions[i].colliderA.SphereCollider->mCollider.origin = posA;

		}
		else if (!collisions[i].colliderA.SphereCollider->mIsDynamic && collisions[i].colliderB.SphereCollider->mIsDynamic)
		{
			vec3f posB = collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->GetPosition() - collisions[i].minimumOverlap;
			collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->SetPosition(posB);
			collisions[i].colliderB.SphereCollider->mCollider.origin = posB;
		}

		collisions[i].colliderA.BaseCollider->OnCollisionExit(collisions[i].colliderB.BaseCollider->mSceneObject);
		collisions[i].colliderB.BaseCollider->OnCollisionExit(collisions[i].colliderA.BaseCollider->mSceneObject);
	}

	collisions.erase(collisions.begin(), collisions.end());
}