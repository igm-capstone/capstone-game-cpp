#include "stdafx.h"
#include "CollisionManager.h"
#include <Rig3D/Intersection.h>
#include "SceneObjects/Explorer.h"
#include "SceneObjects/DominationPoint.h"
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
	std::vector<Collision>& collisions = mCollisions;

	// Get node indices for explorers 
	std::vector<uint32_t> indices;

	indices.reserve(MAX_EXPLORERS);
	
	mBVHTree.GetNodeIndices(indices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER;
	});

	// Explorer / Explorer Collisions

	for (uint32_t i : indices)
	{
		for (uint32_t j : indices)
		{
			if (i == j) continue;

			BVHNode* pNode_e1 = mBVHTree.GetNode(i);
			BVHNode* pNode_e2 = mBVHTree.GetNode(j);

			if (pNode_e1->parentIndex == pNode_e2->parentIndex)
			{
				SphereColliderComponent* pSC_e1 = reinterpret_cast<SphereColliderComponent*>(pNode_e1->object);
				SphereColliderComponent* pSC_e2 = reinterpret_cast<SphereColliderComponent*>(pNode_e2->object);

				if (IntersectSphereSphere(pSC_e1->mCollider, pSC_e2->mCollider))
				{
					float overlap = (pSC_e1->mCollider.radius + pSC_e2->mCollider.radius)
						- magnitude(pSC_e1->mCollider.origin - pSC_e2->mCollider.origin);
					vec3f AtoB = (pSC_e2->mCollider.origin - pSC_e1->mCollider.origin);

					collisions.push_back(Collision());

					Collision* pCollision = &collisions.back();
					pCollision->colliderA.SphereCollider = pSC_e1;
					pCollision->colliderB.SphereCollider = pSC_e2;
					pCollision->minimumOverlap = -AtoB * overlap;

					pSC_e1->OnCollisionEnter(pSC_e2->mSceneObject, overlap);
				}
			}
		}
	}

	// Minion / Minion Collisions

	// Explorer / Minion Collisions

	// Explorer / Wall Collisions

	std::vector<uint32_t> wallIndices;
	wallIndices.reserve(50);

	for (uint32_t idx : indices)
	{
		BVHNode* pNode = mBVHTree.GetNode(idx);
		int parentIndex = pNode->parentIndex;

		mBVHTree.GetNodeIndices(wallIndices, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex && other.object->mLayer == COLLISION_LAYER_WALL;
		});

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		for (uint32_t nIdx : wallIndices)
		{
			OrientedBoxColliderComponent* pWallObbComponent = reinterpret_cast<OrientedBoxColliderComponent*>(mBVHTree.GetNode(nIdx)->object);

			vec3f cp;

			if (IntersectSphereOBB(pSphereComponent->mCollider, pWallObbComponent->mCollider, cp))
			{
				vec3f d = cp - pSphereComponent->mCollider.origin;
				vec3f r = normalize(d) * pSphereComponent->mCollider.radius;

				collisions.push_back(Collision());

				Collision* pCollision = &collisions.back();
				pCollision->colliderA.SphereCollider = pSphereComponent;
				pCollision->colliderB.OBBCollider = pWallObbComponent;
				pCollision->minimumOverlap = d - r;

				pSphereComponent->OnCollisionEnter(pWallObbComponent->mSceneObject, d - r);
			}
		}

		wallIndices.clear();
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