#include "stdafx.h"
#include "BVHTree.h"
#include "SceneObjects/StaticCollider.h"

#include <Resource.h>
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>
#include <Rig3D/Intersection.h>

static vec3f kDefaultOrientation[3] =
{
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f }

};

BVHTree::BVHTree() : mNodes(nullptr), mNodeCount(0)
{
}


BVHTree::~BVHTree()
{
}

void BVHTree::SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount)
{
	mNodes		= reinterpret_cast<BVHNode*>(_aligned_malloc(sizeof(BVHNode) * nodeCount, alignof(BVHNode)));
	mNodeCount	= nodeCount;

	mOrigin		= origin;
	mExtents	= extents;
}

void BVHTree::Initialize()
{
	AddNode(nullptr, BVH_LAYER_ROOT, mNodeCount++, -1, 0);
}

void BVHTree::Update()
{
	
}

void BVHTree::BuildBoundingVolumeHierarchy()
{
	AddNode(Factory<OrientedBoxColliderComponent>::Create(), BVH_LAYER_ROOT, mNodeCount++, -1, 0);

	OrientedBoxColliderComponent* pOBB = reinterpret_cast<OrientedBoxColliderComponent*>(mNodes[0].object);
	pOBB->mCollider.origin		= mOrigin;
	pOBB->mCollider.halfSize	= mExtents;
	pOBB->mCollider.axis[0]		= kDefaultOrientation[0];
	pOBB->mCollider.axis[1]		= kDefaultOrientation[1];
	pOBB->mCollider.axis[2]		= kDefaultOrientation[2];



	vec3f halfExtents = mExtents * 0.5f;

	AABB<vec3f> quadrants[4];
	quadrants[0] = { { mOrigin.x - halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z }, halfExtents };
	quadrants[1] = { mOrigin + halfExtents, halfExtents };
	quadrants[2] = { mOrigin - halfExtents, halfExtents };
	quadrants[3] = { { mOrigin.x + halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z }, halfExtents };



	for (int i = 0; i < 4; i++)
	{
		for (StaticCollider& wall : Factory<StaticCollider>())
		{
			if (IntersectOBBAABB(wall.mBoxCollider->mCollider, quadrants[i]))
			{
				AddNode(wall.mBoxCollider, BVH_LAYER_WALL, mNodeCount++, 0, 0);
			}
		}
	}
}

void BVHTree::AddNode(BaseColliderComponent* pSceneObject, BVHLayer layer, const int& index, const int& parentIndex, const int& depth)
{
	mNodes[index].object		= pSceneObject;
	mNodes[index].layer			= layer;
	mNodes[index].parentIndex	= parentIndex;
}

void BuildBoundingVolumeHierarchyRecursively(BVHNode* pNode, BaseSceneObject* pSceneObject, const int& index, const int& parentIndex, const int& depth);