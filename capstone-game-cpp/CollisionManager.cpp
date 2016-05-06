#include "stdafx.h"
#include "CollisionManager.h"
#include <Rig3D/Intersection.h>
#include "SceneObjects/Explorer.h"
#include "SceneObjects/DominationPoint.h"
#include <algorithm>
#include "SceneObjects/Region.h"
#include "SceneObjects/Heal.h"
#include "SceneObjects/Explosion.h"
#include "Mathf.h"
#include "SceneObjects/Minion.h"

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

	DetectCollisions(milliseconds);
	ResolveCollisions();
}

void CollisionManager::DetectTriggers(std::vector<Collision>& frameCollisions)
{

#pragma region Explorer - Domination Point	
	// The most domination point triggers we can have is 4. An explorer can only be overlapping one domination point at a time.
	frameCollisions.reserve(MAX_EXPLORERS + MAX_EXPLORER_SKILLS);

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
#pragma endregion 

#pragma region Explorer - Heal	

	for (Explorer& e : Factory<Explorer>())
	{
		for (Heal& h : Factory<Heal>())
		{
			if (IntersectSphereSphere(e.mCollider->mCollider, h.mSphereColliderComponent->mCollider))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.SphereCollider = e.mCollider;
				pTrigger->colliderB.SphereCollider = h.mSphereColliderComponent;
			}
		}

		for (Explosion& h : Factory<Explosion>())
		{
			if (IntersectSphereSphere(e.mCollider->mCollider, h.mSphereColliderComponent->mCollider))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.SphereCollider = e.mCollider;
				pTrigger->colliderB.SphereCollider = h.mSphereColliderComponent;
			}
		}
	}

#pragma endregion 

	std::vector<uint32_t> explorerIndices;
	explorerIndices.reserve(MAX_EXPLORERS);
	mBVHTree.GetNodeIndices(explorerIndices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER;
	});

#pragma region Explorer - Interactable

	// Explorer / Doors&Lamps

	std::vector<uint32_t> interactIndices;
	interactIndices.reserve(MAX_DOORS + MAX_LAMPS);

	for (uint32_t i : explorerIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(i);
		int parentIndex = pNode->parentIndex;

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		// Explorer / Door
		mBVHTree.GetNodeIndices(interactIndices, COLLISION_LAYER_INTERACTABLE, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});

		for (uint32_t s : interactIndices)
		{
			BVHNode* pDoorNode = mBVHTree.GetNode(s);

			if (pDoorNode->object->mOnSphereTest(pDoorNode->object, pSphereComponent))
			{
				frameCollisions.push_back(Collision());
				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.BaseCollider = pDoorNode->object;
				pTrigger->colliderB.SphereCollider = pSphereComponent;
			}
		}
	}

#pragma endregion 

#pragma region Explorer - Minion Skill
	// Explorer / Minion Skills

	std::vector<uint32_t> minionSkills;
	minionSkills.reserve(MAX_MINIONS);

	for (uint32_t i : explorerIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(i);
		int parentIndex = pNode->parentIndex;

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		mBVHTree.GetNodeIndices(minionSkills, COLLISION_LAYER_MINION_SKILL, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});

		for (uint32_t ms : minionSkills)
		{
			BVHNode* pMinionSkillNode = mBVHTree.GetNode(ms);

			if (pMinionSkillNode->object->mOnSphereTest(pMinionSkillNode->object, pSphereComponent))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.BaseCollider = pMinionSkillNode->object;
				pTrigger->colliderB.SphereCollider = pSphereComponent;
			}
		}
	}

#pragma endregion 

#pragma region Explorer - Explorer Skill
	// Explorer / Explorer Skill

	std::vector<uint32_t> skillIndices;
	skillIndices.reserve(MAX_EXPLORERS + MAX_EXPLORERS * MAX_EXPLORER_SKILLS); // this is  a guess...

	for (uint32_t i : explorerIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(i);
		int parentIndex = pNode->parentIndex;

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		mBVHTree.GetNodeIndices(skillIndices, COLLISION_LAYER_EXPLORER_SKILL, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});

		for (uint32_t s : skillIndices)
		{
			BVHNode* pSkillNode = mBVHTree.GetNode(s);

			if (pSkillNode->object->mOnSphereTest(pSkillNode->object, pSphereComponent))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.BaseCollider = pSkillNode->object;
				pTrigger->colliderB.SphereCollider = pSphereComponent;
			}
		}

		skillIndices.clear();
	}

#pragma endregion 

#pragma region Explorer Skill - Minions

	mBVHTree.GetNodeIndices(skillIndices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER_SKILL;
	});

	std::vector<uint32_t> minionIndices;
	minionIndices.reserve(MAX_EXPLORER_SKILLS); // Just trying to avoid allocations within the loop.

	for (uint32_t es : skillIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(es);
		int parentIndex = pNode->parentIndex;

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		mBVHTree.GetNodeIndices(minionIndices, COLLISION_LAYER_MINION, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});

		for (uint32_t m : minionIndices)
		{
			BVHNode* pSkillNode = mBVHTree.GetNode(m);

			if (pSkillNode->object->mOnSphereTest(pSkillNode->object, pSphereComponent))
			{
				frameCollisions.push_back(Collision());

				Collision* pTrigger = &frameCollisions.back();
				pTrigger->colliderA.BaseCollider = pSkillNode->object;
				pTrigger->colliderB.SphereCollider = pSphereComponent;
			}
		}

		minionIndices.clear();
	}

#pragma endregion 
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

void CollisionManager::DetectCollisions(double milliseconds)
{
	std::vector<Collision>& collisions = mCollisions;

	// Get node indices for explorers 
	std::vector<uint32_t> explorerIndices;
	explorerIndices.reserve(MAX_EXPLORERS);
	mBVHTree.GetNodeIndices(explorerIndices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER;
	});

	std::vector<uint32_t> minionIndices;
	minionIndices.reserve(MAX_MINIONS);
	mBVHTree.GetNodeIndices(minionIndices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_MINION;
	});

	for (uint32_t i : explorerIndices)
	{
		// Explorer / Explorer Collisions

		for (uint32_t j : explorerIndices)
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
		
		// Explorer / Minion Collisions

		for (uint32_t j : minionIndices)
		{
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

	// Explorer / Wall Collisions

	std::vector<uint32_t> colliderIndices;
	colliderIndices.reserve(50);

	for (uint32_t idx : explorerIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(idx);
		int parentIndex = pNode->parentIndex;

		mBVHTree.GetNodeIndices(colliderIndices, COLLISION_LAYER_WALL, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});

		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);

		for (uint32_t nIdx : colliderIndices)
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

		colliderIndices.clear();
	}

	// Floor Overlaps
	std::vector<uint32_t> allIndices;
	allIndices.reserve(MAX_MINIONS);

	mBVHTree.GetNodeIndices(allIndices, [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER || other.object->mLayer == COLLISION_LAYER_MINION;
	});

	for (uint32_t idx : allIndices)
	{
		BVHNode* pNode = mBVHTree.GetNode(idx);
		int parentIndex = pNode->parentIndex;
		mBVHTree.GetNodeIndices(colliderIndices, COLLISION_LAYER_FLOOR, [parentIndex](const BVHNode& other)
		{
			return other.parentIndex == parentIndex;
		});
	
		SphereColliderComponent* pSphereComponent = reinterpret_cast<SphereColliderComponent*>(pNode->object);
		vec3f position = pNode->object->mSceneObject->mTransform->GetPosition();
		
		Ray<vec3f> ray;
		ray.origin = position + vec3f(0, 0, -2);
		ray.origin.z -= pSphereComponent->mCollider.radius;
		ray.normal = { 0.0f, 0.0f, 1.0f };

		float minZ = FLT_MAX;
		Region* pMinRegion = nullptr;

		for (uint32_t nIdx : colliderIndices)
		{
			Region* pRegion = reinterpret_cast<Region*>(mBVHTree.GetNode(nIdx)->object->mSceneObject);

			vec3f poi;
			float t;

			if (IntersectRayOBB(ray, pRegion->mColliderComponent->mCollider, poi, t))
			{
				if (poi.z < minZ)
				{
					minZ = poi.z;
					pMinRegion = pRegion;
				}
			}
		}

		if (pMinRegion)
		{
			position.z = Mathf::Lerp(position.z, minZ, min(float(milliseconds) * 0.01f, 1));
			pSphereComponent->mSceneObject->mTransform->SetPosition(position);
			pNode->object->OnCollisionExit(pMinRegion);
		}

		colliderIndices.clear();
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
		}
		else if (collisions[i].colliderA.SphereCollider->mIsDynamic && !collisions[i].colliderB.SphereCollider->mIsDynamic)
		{
			vec3f posA = collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->GetPosition() + collisions[i].minimumOverlap;
			collisions[i].colliderA.SphereCollider->mSceneObject->mTransform->SetPosition(posA);
		}
		else if (!collisions[i].colliderA.SphereCollider->mIsDynamic && collisions[i].colliderB.SphereCollider->mIsDynamic)
		{
			vec3f posB = collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->GetPosition() - collisions[i].minimumOverlap;
			collisions[i].colliderB.SphereCollider->mSceneObject->mTransform->SetPosition(posB);
		}

		collisions[i].colliderA.BaseCollider->OnCollisionExit(collisions[i].colliderB.BaseCollider->mSceneObject);
		collisions[i].colliderB.BaseCollider->OnCollisionExit(collisions[i].colliderA.BaseCollider->mSceneObject);
	}

	collisions.erase(collisions.begin(), collisions.end());
}