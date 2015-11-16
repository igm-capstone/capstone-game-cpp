#pragma once
#include "Rig3D/Intersection.h"
#include <vector>
#include <utility>

using namespace cliqCity::graphicsMath;

template<class Vector>
struct RayCastHit
{
	Vector			poi;		// Point of intersection
	float			distance;	// Distance 
	uint32_t		index;		// Index of collider
};

template <class Vector>
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, AABB<Vector>* aabbs, uint32_t count)
{
	std::vector<RayCastHit<Vector>> hits;
	hits.reserve(count);

	for (uint32_t i = 0; i < count; i++)
	{
		Vector poi;
		float t;
		if (IntersectRayAABB(ray, aabbs[i], poi, t))
		{
			hits.push_back({ poi , 0.0f, i });	// Temp store zero for distance to save on calculations
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	int minIndex = 0;

	float prevDistance = magnitudeSquared(aabbs[hits.at(0).index].origin - ray.origin);
	for (uint32_t i = 1; i < hits.size(); i++)
	{
		float distance = magnitudeSquared(aabbs[hits.at(i).index].origin - ray.origin);
		if (distance < prevDistance)
		{
			hits.at(i).distance = sqrt(distance);
			minIndex = i;
			prevDistance = distance;
		}
	}

	*hitInfo = hits.at(minIndex);

	return 1;
}

template <class Vector>
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, AABB<Vector>* aabbs, uint32_t count, float maxDistance)
{
	std::vector<RayCastHit<Vector>> hits;
	hits.reserve(count);

	Line<Vector> line{ ray.origin, ray.origin + (ray.normal * maxDistance) };

	for (uint32_t i = 0; i < count; i++)
	{
		Vector poi;
		float t;
		if (IntersectLineAABB(line, aabbs[i], poi, t))
		{
			hits.push_back({ poi , 0.0f, i });	// Temp store zero for distance to save on calculations
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	float minIndex = 0;

	float prevDistance = magnitudeSquared(aabbs[hits.at(0).index].origin - ray.origin);
	for (uint32_t i = 1; i < hits.size(); i++)
	{
		float distance = magnitudeSquared(aabbs[hits.at(i).index].origin - ray.origin);
		if (distance < prevDistance)
		{
			hits.at(i).distance = sqrt(distance);
			minIndex = i;
			prevDistance = distance;
		}
	}

	*hitInfo = hits.at(minIndex);

	return 1;
}

template<class Vector>
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, Sphere<Vector>* spheres, uint32_t count)
{
	uint32_t	minIndex = 0;
	float		tMin = FLT_MAX;

	for (uint32_t i = 0; i < count; i++)
	{
		Vector poi;
		float t;
		if (IntersectRaySphere(ray, spheres[i], poi, t))
		{
			if (t < tMin) {
				tMin = t;
				minIndex = i;
			}
		}

	}

	// No hits
	if (tMin == FLT_MAX)
	{
		return 0;
	}

	// Calculate stats for hit
	hitInfo->index = minIndex;
	hitInfo->poi = ray.origin + ray.normal * tMin;
	hitInfo->distance = cliqCity::graphicsMath::magnitude(spheres[minIndex].origin - ray.origin);

	return 1;
}