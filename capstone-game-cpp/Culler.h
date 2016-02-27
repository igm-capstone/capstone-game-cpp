#pragma once
#include <Rig3D/Visibility.h>
#include "SceneObjects/StaticCollider.h"

template <class SceneObjectType>
inline void CullOBBSceneObjects(const Rig3D::Frustum& frustum, std::vector<BaseSceneObject*>& pBaseSceneObjects, std::vector<uint32_t>& indices)
{
	float distance;
	const Plane<vec3f>* planes[6] =
	{
		&frustum.front,
		&frustum.back,
		&frustum.left,
		&frustum.right,
		&frustum.bottom,
		&frustum.top,
	};

	for (uint32_t idx = 0; idx < pBaseSceneObjects.size(); idx++)
	{
		SceneObjectType* pSceneObject = reinterpret_cast<SceneObjectType*>(pBaseSceneObjects[idx]);

		bool shouldCull = false;
		for (uint32_t p = 0; p < 6; p++)
		{
			float pir =
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[0] * abs(dot(planes[p]->normal, pSceneObject->mColliderComponent->mCollider.axis[0])) +
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[1] * abs(dot(planes[p]->normal, pSceneObject->mColliderComponent->mCollider.axis[1])) +
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[2] * abs(dot(planes[p]->normal, pSceneObject->mColliderComponent->mCollider.axis[2]));

			// Compute distance from obb center to plane
			distance = dot(planes[p]->normal, pSceneObject->mColliderComponent->mCollider.origin) - planes[p]->distance;
			if (distance < -pir)
			{
				shouldCull = true;
				break;
			}
		}

		if (!shouldCull)
		{
			indices.push_back(idx);
		}
	}
}

template <class SceneObjectType>
inline void CullAABBSceneObjects(const Rig3D::Frustum& frustum, std::vector<BaseSceneObject*>& pBaseSceneObjects, std::vector<uint32_t>& indices)
{
	float distance;
	const Plane<vec3f>* planes[6] =
	{
		&frustum.front,
		&frustum.back,
		&frustum.left,
		&frustum.right,
		&frustum.bottom,
		&frustum.top,
	};

	for (uint32_t idx = 0; idx < pBaseSceneObjects.size(); idx++)
	{
		SceneObjectType* pSceneObject = reinterpret_cast<SceneObjectType*>(pBaseSceneObjects[idx]);

		bool shouldCull = false;
		for (uint32_t p = 0; p < 6; p++)
		{
			float pir =
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[0] * abs(planes[p]->normal.pCols[0]) +
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[1] * abs(planes[p]->normal.pCols[1]) +
				pSceneObject->mColliderComponent->mCollider.halfSize.pCols[2] * abs(planes[p]->normal.pCols[2]);

			// Compute distance from obb center to plane
			distance = dot(planes[p]->normal, pSceneObject->mColliderComponent->mCollider.origin) - planes[p]->distance;
			if (distance < -pir)
			{
				shouldCull = true;
				break;
			}
		}

		if (!shouldCull)
		{
			indices.push_back(idx);
		}
	}
}

inline void CullPlanes(const Rig3D::Frustum& frustum, std::vector<uint32_t>& indices, mat4f* planeWorldMatrices, float planeWidth, float planeHeight,
	const uint32_t& count)
{
	float distance;
	const Plane<vec3f>* planes[6] =
	{
		&frustum.front,
		&frustum.back,
		&frustum.left,
		&frustum.right,
		&frustum.bottom,
		&frustum.top,
	};

	float halfPlaneWidth = planeWidth * 0.5f;
	float halfPlaneHeight = planeHeight * 0.5f;

	for (uint32_t i = 0; i < count;i++)
	{
		AABB<vec3f> aabb;
		aabb.origin = planeWorldMatrices[i].transpose().t;
		aabb.halfSize = { halfPlaneWidth, halfPlaneHeight, 0.5f };

		bool shouldCull = false;
		for (uint32_t p = 0; p < 6; p++)
		{
			float pir =
				aabb.halfSize.pCols[0] * abs(planes[p]->normal.pCols[0]) +
				aabb.halfSize.pCols[1] * abs(planes[p]->normal.pCols[1]) +
				aabb.halfSize.pCols[2] * abs(planes[p]->normal.pCols[2]);

			distance = dot(planes[p]->normal, aabb.origin) - planes[p]->distance;

			// Compute distance from obb center to plane
			distance = dot(planes[p]->normal, aabb.origin) - planes[p]->distance;
			if (distance < -pir)
			{
				shouldCull = true;
				break;
			}
		}

		if (!shouldCull)
		{
			indices.push_back(i);
		}
	}
}

