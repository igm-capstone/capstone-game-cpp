#include "stdafx.h"
#include "BVHTree.h"
#include "SceneObjects/StaticCollider.h"
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>
#include <Rig3D/Intersection.h>
#include <trace.h>
#include <Colors.h>
#include "SceneObjects/Explorer.h"


#define PARTITION_COUNT 4

#define WALL_PARENT_LAYER_INDEX		1
#define EXPLORER_PARENT_LAYER_INDEX 1

inline int OBBComponentTest(BaseColliderComponent* a, OrientedBoxColliderComponent* b)
{
	return IntersectOBBOBB(reinterpret_cast<OrientedBoxColliderComponent*>(a)->mCollider, b->mCollider);
}

inline int SphereComponentTest(BaseColliderComponent* a, OrientedBoxColliderComponent* b)
{
	vec3f cp;
	return IntersectSphereOBB(reinterpret_cast<SphereColliderComponent*>(a)->mCollider, b->mCollider, cp);
}

struct BVHNodeLayerFilter
{
	CLayer					layer;

	BVHNodeLayerFilter(CLayer layer) : layer(layer)
	{
		
	}

	bool operator()(const BVHNode& other) const
	{

		return layer == other.object->mLayer;
	}
};

struct BVHNodeIntesection
{
	CLayer	layer;
	int		parentIndex;
	
};

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
		COLLISION_LAYER_FLOOR,
		COLLISION_LAYER_WALL,
		COLLISION_LAYER_EXPLORER,
		COLLISION_LAYER_MINION,
		COLLISION_LAYER_SKILL,
		COLLISION_LAYER_LAMP
	};
}

BVHTree::BVHTree()
{
}

BVHTree::~BVHTree()
{
}

void BVHTree::SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount)
{
	mNodes.reserve(nodeCount + PARTITION_COUNT + 1 + MAX_EXPLORERS);
	mOrigin		= origin;
	mExtents	= extents;
}

void BVHTree::Initialize()
{
	BuildBoundingVolumeHierarchy();
}

void BVHTree::Update()
{
	RenderDebug();

	mNodes.erase(std::remove_if(mNodes.begin(), mNodes.end(), [](const BVHNode& other)
	{
		return other.object->mLayer == COLLISION_LAYER_EXPLORER || other.object->mLayer == COLLISION_LAYER_MINION;
	}), mNodes.end());

	for (Explorer& explorer : Factory<Explorer>())
	{
		AddNodeRecursively(explorer.mCollider, EXPLORER_PARENT_LAYER_INDEX, 1, 0, 0, SphereComponentTest);
	}
}

void BVHTree::BuildBoundingVolumeHierarchy()
{
	OrientedBoxColliderComponent* pOBB = Factory<OrientedBoxColliderComponent>::Create();
	pOBB->mCollider.origin = mOrigin;
	pOBB->mCollider.halfSize = { mExtents.x, mExtents.y, 50.0f };
	pOBB->mCollider.axis[0] = kDefaultOrientation[0];
	pOBB->mCollider.axis[1] = kDefaultOrientation[1];
	pOBB->mCollider.axis[2] = kDefaultOrientation[2];
	pOBB->mSceneObject = this;

	AddNode(pOBB, -1, 0);	

	vec3f halfExtents = mExtents * 0.5f;



	vec3f quadrants[PARTITION_COUNT];
	quadrants[0] = { mOrigin.x - halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[1] = { mOrigin.x + halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[2] = { mOrigin.x - halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };
	quadrants[3] = { mOrigin.x + halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };

	TRACE("QUADRANTS" << Trace::endl);

	for (int i = 1; i < PARTITION_COUNT + 1; i++)
	{
		TRACE(" " << i - 1 << Trace::endl);

		pOBB = Factory<OrientedBoxColliderComponent>::Create();
		pOBB->mCollider.origin = quadrants[i - 1];
		pOBB->mCollider.halfSize = {halfExtents.x, halfExtents.y, 50.0f};
		pOBB->mCollider.axis[0] = kDefaultOrientation[0];
		pOBB->mCollider.axis[1] = kDefaultOrientation[1];
		pOBB->mCollider.axis[2] = kDefaultOrientation[2];
		pOBB->mLayer = COLLISION_LAYER_QUADRANT;
		pOBB->mSceneObject = this;

		AddNodeRecursively(pOBB, 0, 0, 0, 0, OBBComponentTest);
	}

	//for (StaticCollider& collider : Factory<StaticCollider>())
	//{
	//	if (collider.mBoxCollider->mLayer == COLLISION_LAYER_FLOOR)
	//	{
	//		AddNodeRecursively(collider.mBoxCollider, 1, 0, 0, 0, OBBComponentTest);
	//	}
	//}

	for (StaticCollider& collider : Factory<StaticCollider>())
	{
		if (collider.mBoxCollider->mLayer == COLLISION_LAYER_WALL)
		{
			AddNodeRecursively(collider.mBoxCollider, WALL_PARENT_LAYER_INDEX, 0, 0, 0, OBBComponentTest);
		}
	}
}

void BVHTree::AddNode(BaseColliderComponent* pColliderComponent, const int& parentIndex, const int& depth)
{
	mNodes.push_back(BVHNode());

	BVHNode* pNode = &mNodes.back();

	pNode->object		= pColliderComponent;
	pNode->parentIndex	= parentIndex;
}

void BVHTree::AddNodeRecursively(BaseColliderComponent* pColliderComponent, const int& parentLayerIndex, const int& layerIndex, const int& parentIndex, const int& depth, IntersectionTest intersectionTest)
{
	for (uint32_t i = parentIndex; i < mNodes.size(); i++)
	{
		if (mNodes[i].object->mLayer == gAllLayers[layerIndex])
		{
			OrientedBoxColliderComponent* pOBB = reinterpret_cast<OrientedBoxColliderComponent*>(mNodes[i].object);

			vec3f cp;
			if (intersectionTest(pColliderComponent, pOBB))
			{
				if (mNodes[i].object->mLayer != gAllLayers[parentLayerIndex])
				{
					AddNodeRecursively(pColliderComponent, parentLayerIndex, layerIndex + 1, static_cast<int>(i), depth + 1, intersectionTest);
				}
				else
				{
					for (int i = 0; i < depth; i++)
					{
						TRACE(" ");
					}

					TRACE("Adding node to " << pOBB->mLayer << " " << pOBB->mCollider.origin << Trace::endl);
					AddNode(pColliderComponent, static_cast<int>(i), layerIndex);
				}
			}
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

void BVHTree::RenderDebug()
{
	vec3f halfExtents = mExtents * 0.5f;

	vec3f quadrants[PARTITION_COUNT];
	quadrants[0] = { mOrigin.x - halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[1] = { mOrigin.x + halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[2] = { mOrigin.x - halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };
	quadrants[3] = { mOrigin.x + halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };

	vec3f corners[8];
	corners[0] = { quadrants[0].x - halfExtents.x, quadrants[0].y + halfExtents.y, 0 };	// TL
	corners[1] = { quadrants[0].x + halfExtents.x, quadrants[0].y + halfExtents.y, 0 };	// TM

	corners[2] = { quadrants[1].x + halfExtents.x, quadrants[1].y + halfExtents.y, 0 }; // TR
	corners[3] = { quadrants[1].x + halfExtents.x, quadrants[1].y - halfExtents.y, 0 }; // MR

	corners[4] = { quadrants[3].x + halfExtents.x, quadrants[3].y - halfExtents.y, 0 }; // BR
	corners[5] = { quadrants[3].x - halfExtents.x, quadrants[3].y - halfExtents.y, 0 }; // BM

	corners[6] = { quadrants[2].x - halfExtents.x, quadrants[2].y - halfExtents.y, 0 }; // BL
	corners[7] = { quadrants[2].x - halfExtents.x, quadrants[2].y + halfExtents.y, 0 }; // ML

	TRACE_LINE(corners[0], corners[2], Colors::red);
	TRACE_LINE(corners[2], corners[4], Colors::red);
	TRACE_LINE(corners[4], corners[6], Colors::red);
	TRACE_LINE(corners[6], corners[0], Colors::red);

	TRACE_LINE(corners[1], corners[5], Colors::red);
	TRACE_LINE(corners[7], corners[3], Colors::red);
}