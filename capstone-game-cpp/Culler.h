#pragma once
#include <Rig3D/Visibility.h>
#include <SceneObjects/Wall.h>

inline void CullWalls(const Rig3D::Frustum& frustum, std::vector<uint32_t>& indices)
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

	uint32_t index = 0;
	for (Wall& w : Factory<Wall>())
	{
		bool shouldCull = false;
		for (uint32_t p = 0; p < 6; p++)
		{
			float pir =
				w.mBoxCollider->mCollider.halfSize.pCols[0] * abs(cliqCity::graphicsMath::dot(planes[p]->normal, w.mBoxCollider->mCollider.axis[0])) +
				w.mBoxCollider->mCollider.halfSize.pCols[1] * abs(cliqCity::graphicsMath::dot(planes[p]->normal, w.mBoxCollider->mCollider.axis[1])) +
				w.mBoxCollider->mCollider.halfSize.pCols[2] * abs(cliqCity::graphicsMath::dot(planes[p]->normal, w.mBoxCollider->mCollider.axis[2]));

			// Compute distance from obb center to plane
			distance = cliqCity::graphicsMath::dot(planes[p]->normal, w.mBoxCollider->mCollider.origin) - planes[p]->distance;			
			if (distance < -pir)
			{
				shouldCull = true;
				break;
			}
		}

		if (!shouldCull)
		{
			indices.push_back(index);
		}

		index++;
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

			distance = cliqCity::graphicsMath::dot(planes[p]->normal, aabb.origin) - planes[p]->distance;

			// Compute distance from obb center to plane
			distance = cliqCity::graphicsMath::dot(planes[p]->normal, aabb.origin) - planes[p]->distance;
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

