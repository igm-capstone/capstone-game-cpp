#include "stdafx.h"
#include "BVHTree.h"
#include "SceneObjects/StaticCollider.h"
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>
#include <Rig3D/Intersection.h>
#include <trace.h>
#include <Colors.h>

#define PARTITION_COUNT 4

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
	mNodes.reserve(nodeCount + PARTITION_COUNT + 1);
	mOrigin		= origin;
	mExtents	= extents;
}

void BVHTree::Initialize()
{
	BuildBoundingVolumeHierarchy();
}

void BVHTree::Update()
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

void BVHTree::BuildBoundingVolumeHierarchy()
{
	OrientedBoxColliderComponent* pOBB = Factory<OrientedBoxColliderComponent>::Create();
	pOBB->mCollider.origin = mOrigin;
	pOBB->mCollider.halfSize = mExtents;
	pOBB->mCollider.axis[0] = kDefaultOrientation[0];
	pOBB->mCollider.axis[1] = kDefaultOrientation[1];
	pOBB->mCollider.axis[2] = kDefaultOrientation[2];
	pOBB->mSceneObject = this;

	AddNode(pOBB, mNodes.size(), -1, 0);	

	vec3f halfExtents = mExtents * 0.5f;

	vec3f quadrants[PARTITION_COUNT];
	quadrants[0] = { mOrigin.x - halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[1] = { mOrigin.x + halfExtents.x, mOrigin.y + halfExtents.y, mOrigin.z };
	quadrants[2] = { mOrigin.x - halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };
	quadrants[3] = { mOrigin.x + halfExtents.x, mOrigin.y - halfExtents.y, mOrigin.z };

	vec3f corners[9];
	corners[0] = { quadrants[0].x - halfExtents.x, quadrants[0].y + halfExtents.y, 0 };	// TL
	corners[1] = { quadrants[0].x + halfExtents.x, quadrants[0].y + halfExtents.y, 0 };	// TM

	corners[2] = { quadrants[1].x + halfExtents.x, quadrants[1].y + halfExtents.y, 0 }; // TR
	corners[3] = { quadrants[1].x + halfExtents.x, quadrants[1].y - halfExtents.y, 0 }; // MR

	corners[4] = { quadrants[3].x + halfExtents.x, quadrants[3].y - halfExtents.y, 0 }; // BR
	corners[5] = { quadrants[3].x - halfExtents.x, quadrants[3].y + halfExtents.y, 0 }; // BM

	corners[6] = { quadrants[2].x - halfExtents.x, quadrants[2].y - halfExtents.y, 0 }; // BL
	corners[6] = { quadrants[2].x - halfExtents.x, quadrants[2].y + halfExtents.y, 0 }; // ML

	





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
		pOBB->mSceneObject = this;

		AddNode(pOBB, mNodes.size(), 0, 1);
		AddNodes(&pOBB->mCollider, 1, mNodes.size(), 1, 2);
	}
}

void BVHTree::AddNodes(OrientedBoxCollider* boundingVolume, const int& layerIndex, const int& index, const int& parentIndex, const int& depth)
{
	for (StaticCollider& collider : Factory<StaticCollider>())
	{
		if (collider.mBoxCollider->mLayer == gAllLayers[layerIndex])
		{
			if (IntersectOBBOBB(collider.mBoxCollider->mCollider, *boundingVolume))
			{
				for (int i = 0; i < depth;i++)
				{
					TRACE(" ");
				}

				TRACE("POS: " << collider.mBoxCollider->mCollider.origin << Trace::endl);

				AddNode(collider.mBoxCollider, mNodes.size(), index, depth + 1);
				AddNodes(&collider.mBoxCollider->mCollider,  layerIndex + 1, mNodes.size(), index, depth + 1);
			}
		}
	}
}

void BVHTree::AddNode(BaseColliderComponent* pSceneObject, const int& index, const int& parentIndex, const int& depth)
{
	mNodes.push_back(BVHNode());

	BVHNode* pNode = &mNodes.back();

	pNode->object		= pSceneObject;
	pNode->parentIndex	= parentIndex;
}