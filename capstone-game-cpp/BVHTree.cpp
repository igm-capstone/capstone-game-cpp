#include "stdafx.h"
#include "BVHTree.h"
#include "SceneObjects/StaticCollider.h"
#include "SceneObjects/Region.h"
#include <SceneObjects/BaseSceneObject.h>
#include <Components/Skill.h>
#include <Components/ColliderComponent.h>
#include <Rig3D/Intersection.h>
#include <Colors.h>
#include "SceneObjects/Explorer.h"
#include "SceneObjects/Minion.h"
#include "SceneObjects/Door.h"
#include "SceneObjects/StaticMesh.h"
#include "SceneObjects/Trap.h"
#include "SceneObjects/Lamp.h"
#include "SceneObjects/Lantern.h"

#define PARTITION_X_COUNT 3
#define PARTITION_Y_COUNT 2

#define WALL_PARENT_LAYER_INDEX		1
#define EXPLORER_PARENT_LAYER_INDEX 1
#define REGION_PARENT_LAYER_INDEX   1
#define SKILL_PARENT_LAYER_INDEX	1
#define DOOR_PARENT_LAYER_INDEX		1

#ifdef _DEBUG
#include <trace.h>

void RenderDebug(BVHTree& bvhTree)
{
	vec3f tl = { bvhTree.mOrigin.x - bvhTree.mExtents.x, bvhTree.mOrigin.y + bvhTree.mExtents.y, 0.0f };
	vec3f tr = { bvhTree.mOrigin.x + bvhTree.mExtents.x, bvhTree.mOrigin.y + bvhTree.mExtents.y, 0.0f };
	vec3f bl = { bvhTree.mOrigin.x - bvhTree.mExtents.x, bvhTree.mOrigin.y - bvhTree.mExtents.y, 0.0f };

	TRACE_LINE(tl, tr, Colors::red);
	TRACE_LINE(tl, bl, Colors::red);


	vec2f traceStep = bvhTree.mQuadrantExtents * 2.0f;

	for (int x = 0; x < PARTITION_X_COUNT; x++)
	{
		OrientedBoxColliderComponent* pObb = reinterpret_cast<OrientedBoxColliderComponent*>(bvhTree.GetNode(x + 1)->object);
		vec3f qtr = { pObb->mCollider.origin.x + pObb->mCollider.halfSize.x, pObb->mCollider.origin.y + pObb->mCollider.halfSize.y,0.0f };
		vec3f obr = { pObb->mCollider.origin.x + pObb->mCollider.halfSize.x, bl.y, 0.0f };
		TRACE_LINE(qtr, obr, Colors::red);
	}

	for (int y = 0; y < PARTITION_Y_COUNT; y++)
	{
		OrientedBoxColliderComponent* pObb = reinterpret_cast<OrientedBoxColliderComponent*>(bvhTree.GetNode((y * PARTITION_X_COUNT) + 1)->object);
		vec3f qbl = { pObb->mCollider.origin.x - pObb->mCollider.halfSize.x, pObb->mCollider.origin.y - pObb->mCollider.halfSize.y, 0.0f };
		vec3f obr = { tr.x, pObb->mCollider.origin.y - pObb->mCollider.halfSize.y, 0.0f };
		TRACE_LINE(qbl, obr, Colors::red);
	}
}
#endif

static vec3f kDefaultOrientation[3] =
{
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f }
};

namespace
{
	static const CLayer gAllLayers[] =
	{
		COLLISION_LAYER_ROOT,
		COLLISION_LAYER_QUADRANT,
		COLLISION_LAYER_EXPLORER,
		COLLISION_LAYER_INTERACTABLE,
		COLLISION_LAYER_FLOOR,
		COLLISION_LAYER_WALL,
		COLLISION_LAYER_MINION,
		COLLISION_LAYER_EXPLORER_SKILL
	};
}

BVHTree::BVHTree()
{
}

BVHTree::~BVHTree()
{
	mNodes.clear();
	mLayerStartIndex.clear();

	if (mNodes.size())
	{
		for (int i = 0; i < (PARTITION_X_COUNT * PARTITION_Y_COUNT) + 1; i++)
		{
			Factory<OrientedBoxColliderComponent>::Destroy(reinterpret_cast<OrientedBoxColliderComponent*>(mNodes[i].object));
		}
	}
}

void BVHTree::SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount)
{
	mNodes.reserve(MAX_STATIC_COLLIDERS);
	mOrigin		= origin;
	mExtents	= extents;
}

void BVHTree::Initialize()
{
	BuildBoundingVolumeHierarchy();
}

void BVHTree::Update()
{
#ifdef _DEBUG
	if (gDebugBVH) RenderDebug(*this);
#endif 

	mLayerStartIndex.erase(COLLISION_LAYER_EXPLORER);
	mLayerStartIndex.erase(COLLISION_LAYER_MINION);
	mLayerStartIndex.erase(COLLISION_LAYER_EXPLORER_SKILL);
	mLayerStartIndex.erase(COLLISION_LAYER_MINION_SKILL);


	mNodes.erase(std::remove_if(mNodes.begin(), mNodes.end(), [](const BVHNode& other)
	{
		return 
			other.object->mLayer == COLLISION_LAYER_EXPLORER || 
			other.object->mLayer == COLLISION_LAYER_MINION ||
			other.object->mLayer == COLLISION_LAYER_EXPLORER_SKILL ||
			other.object->mLayer == COLLISION_LAYER_MINION_SKILL;
	}), mNodes.end());

	// Add Explorers and explorer skills
	for (Explorer& explorer : Factory<Explorer>())
	{
		AddNodeRecursively(explorer.mCollider, EXPLORER_PARENT_LAYER_INDEX, 1, 0, 0);

		if (explorer.mMeleeColliderComponent.asBaseColliderComponent && explorer.mMeleeColliderComponent.asBaseColliderComponent->mIsActive)
		{
			AddNodeRecursively(explorer.mMeleeColliderComponent.asBaseColliderComponent, SKILL_PARENT_LAYER_INDEX, 1, 0, 0);
		}

		if (explorer.mInteractionCollider->mIsActive)
		{
			AddNodeRecursively(explorer.mInteractionCollider, SKILL_PARENT_LAYER_INDEX, 1, 0, 0);
		}
	}

	// Add Minion and minion skills.
	for (Minion& minion : Factory<Minion>())
	{
		AddNodeRecursively(minion.mCollider, EXPLORER_PARENT_LAYER_INDEX, 1, 0, 0);
	
		if (minion.mMeleeColliderComponent->mIsActive)
		{
			AddNodeRecursively(minion.mMeleeColliderComponent, SKILL_PARENT_LAYER_INDEX, 1, 0, 0);
		}
	}

	for (Trap& trap : Factory<Trap>())
	{
		if (trap.mSphereColliderComponent->mIsActive)
		{
			AddNodeRecursively(trap.mSphereColliderComponent, EXPLORER_PARENT_LAYER_INDEX, 1, 0, 0);
		}
	}

	for (Lantern& lantern : Factory<Lantern>())
	{
		if (lantern.mColliderComponent->mIsActive)
		{
			AddNodeRecursively(lantern.mColliderComponent, EXPLORER_PARENT_LAYER_INDEX, 1, 0, 0);
		}
	}
}

void BVHTree::BuildBoundingVolumeHierarchy()
{
	OrientedBoxColliderComponent* pOBB = Factory<OrientedBoxColliderComponent>::Create();
	pOBB->mCollider.origin = mOrigin;
	pOBB->mCollider.halfSize = { mExtents.x, mExtents.y, 25.0f };
	pOBB->mCollider.axis[0] = kDefaultOrientation[0];
	pOBB->mCollider.axis[1] = kDefaultOrientation[1];
	pOBB->mCollider.axis[2] = kDefaultOrientation[2];
	pOBB->mSceneObject = this;

	AddNode(pOBB, -1, 0);	

	vec2f rootSize			= { mExtents.x * 2.0f, mExtents.y * 2.0f };
	vec2f quadrantSize		= { rootSize.x / static_cast<float>(PARTITION_X_COUNT),  rootSize.y / static_cast<float>(PARTITION_Y_COUNT) };
	vec2f quadrantOffset	= (rootSize - quadrantSize) * 0.5f;
	mQuadrantExtents		= { quadrantSize.x * 0.5f, quadrantSize.y * 0.5f, 25.0f };

	for (int y = 0; y < PARTITION_Y_COUNT; y++)
	{
		for (int x = 0; x < PARTITION_X_COUNT; x++)
		{
			pOBB = Factory<OrientedBoxColliderComponent>::Create();
			pOBB->mCollider.origin = { mOrigin.x + (x * quadrantSize.x - quadrantOffset.x), mOrigin.y + (quadrantOffset.y - quadrantSize.y * y) , 0.0f };
			pOBB->mCollider.halfSize = mQuadrantExtents;
			pOBB->mCollider.axis[0] = kDefaultOrientation[0];
			pOBB->mCollider.axis[1] = kDefaultOrientation[1];
			pOBB->mCollider.axis[2] = kDefaultOrientation[2];
			pOBB->mLayer = COLLISION_LAYER_QUADRANT;
			pOBB->mSceneObject = this;

			AddNodeRecursively(pOBB, 0, 0, 0, 0);
		}
	}

	for (Region& region : Factory<Region>())
	{
		AddNodeRecursively(region.mColliderComponent, REGION_PARENT_LAYER_INDEX, 0, 0, 0);
	}

	for (StaticCollider& collider : Factory<StaticCollider>())
	{
		AddNodeRecursively(collider.mColliderComponent, WALL_PARENT_LAYER_INDEX, 0, 0, 0);
	}

	for (StaticMesh& collider : Factory<StaticMesh>())
	{
		if (collider.mColliderComponent->mIsActive == false) continue;
		AddNodeRecursively(collider.mColliderComponent, WALL_PARENT_LAYER_INDEX, 0, 0, 0);
	}

	for (Door& door : Factory<Door>())
	{
		AddNodeRecursively(door.mColliderComponent, WALL_PARENT_LAYER_INDEX, 0, 0, 0);
		AddNodeRecursively(door.mTrigger, DOOR_PARENT_LAYER_INDEX, 0, 0, 0);
	}

	for (Lamp& door : Factory<Lamp>())
	{
		AddNodeRecursively(door.mTrigger, DOOR_PARENT_LAYER_INDEX, 0, 0, 0);
	}
}

void BVHTree::AddNode(BaseColliderComponent* pColliderComponent, const int& parentIndex, const int& depth)
{
	if (mLayerStartIndex.find(pColliderComponent->mLayer) == mLayerStartIndex.end())
	{
		mLayerStartIndex.insert({ pColliderComponent->mLayer , mNodes.size() });
	}

	mNodes.push_back(BVHNode());

	BVHNode* pNode = &mNodes.back();
	pNode->object		= pColliderComponent;
	pNode->parentIndex	= parentIndex;
}

void BVHTree::AddNodeRecursively(BaseColliderComponent* pColliderComponent, const int& parentLayerIndex, const int& layerIndex, const int& parentIndex, const int& depth)
{
	for (uint32_t i = parentIndex; i < mNodes.size(); i++)
	{
		if (mNodes[i].object->mLayer == gAllLayers[layerIndex])
		{
			OrientedBoxColliderComponent* pOBB = reinterpret_cast<OrientedBoxColliderComponent*>(mNodes[i].object);

			vec3f cp;
			if (pColliderComponent->mOnObbTest(pColliderComponent, pOBB))
			{
				if (mNodes[i].object->mLayer != gAllLayers[parentLayerIndex])
				{
					AddNodeRecursively(pColliderComponent, parentLayerIndex, layerIndex + 1, static_cast<int>(i), depth + 1);
				}
				else
				{
					AddNode(pColliderComponent, static_cast<int>(i), layerIndex);
				}
			}
		}
	}
}

BaseColliderComponent* BVHTree::RayCast(Ray<vec3f> ray, vec3f& poi)
{
	BaseColliderComponent* pCollider = nullptr;
	float tMin = FLT_MAX;

	RayCastRecursively(ray, 0, &pCollider, poi, tMin);
	
	return pCollider;
}

void BVHTree::RayCastRecursively(Ray<vec3f> ray, const int& parentIndex, BaseColliderComponent** pCollider, vec3f& poi, float& tMin)
{
	std::vector<uint32_t> indices;
	GetNodeIndices(indices, [parentIndex](const BVHNode& other)
	{
		return (other.parentIndex == parentIndex &&
			(other.object->mLayer == COLLISION_LAYER_ROOT ||
				other.object->mLayer == COLLISION_LAYER_QUADRANT ||
				other.object->mLayer == COLLISION_LAYER_INTERACTABLE ||
				other.object->mLayer == COLLISION_LAYER_FLOOR ||
				other.object->mLayer == COLLISION_LAYER_EXPLORER));
	});

	float t;

	for (uint32_t index : indices)
	{
		BVHNode* pNode = &mNodes[index];

		if (pNode->object->mOnRayTest(ray, pNode->object, poi, t))
		{
			if (pNode->object->mLayer == COLLISION_LAYER_ROOT ||
				pNode->object->mLayer == COLLISION_LAYER_QUADRANT)
			{
				RayCastRecursively(ray, index, pCollider, poi, tMin);
			}
			else
			{
				if (t < tMin)
				{
					tMin = t;
					*pCollider = pNode->object;
				}
			}
		}
	}
}

void BVHTree::GetNodeIndices(std::vector<uint32_t>& indices, const CLayer& layer, std::function<bool(const BVHNode& other)> predicate)
{
	uint32_t startIndex = 0;
	if (mLayerStartIndex.find(layer) != mLayerStartIndex.end())
	{
		startIndex = mLayerStartIndex[layer];
	}

	BVHNode* pNodes = &mNodes[0];
	uint32_t nodeCount = mNodes.size();

	for (uint32_t i = startIndex; i < nodeCount; i++)
	{
		if (pNodes[i].object->mLayer == layer && predicate(pNodes[i]) && pNodes[i].object->mIsActive)
		{
			indices.push_back(i);
		}
	}
}

void BVHTree::GetNodeIndices(std::vector<uint32_t>& indices, std::function<bool(const BVHNode& other)> predicate)
{
	BVHNode* pNodes		= &mNodes[0];
	uint32_t nodeCount	= mNodes.size();

	for (uint32_t i = 0; i < nodeCount; i++)
	{
		if (predicate(pNodes[i]))
		{
			indices.push_back(i);
		}
	}
}

BVHNode* BVHTree::GetNode(const uint32_t& index)
{
	return &mNodes[index];
}
