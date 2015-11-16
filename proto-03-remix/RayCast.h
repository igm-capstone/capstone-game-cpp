#pragma once
#include "Rig3D/Intersection.h"
#include <vector>
#include <utility>

using namespace cliqCity::graphicsMath;

template<class Vector>
struct RayCastHit
{
	AABB<Vector>*	hit;
	Vector			poi;
	float			distance;
};

template <class Vector>
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, AABB<Vector>* aabbs, int count)
{
	std::vector<RayCastHit<Vector>> hits;
	hits.reserve(count);

	for (int i = 0; i < count; i++)
	{
		Vector poi;
		float t;
		if (IntersectRayAABB(ray, aabbs[i], poi, t))
		{
			hits.push_back({ &aabbs[i], poi , 0.0f });	// Temp store zero for distance to save on calculations
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	float minIndex = 0;

	float prevDistance = magnitudeSquared(hits.at(0).hit->origin - ray.origin);
	for (int i = 1; i < hits.size(); i++)
	{
		float distance = magnitudeSquared(hits.at(i).hit->origin - ray.origin);
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
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, AABB<Vector>* aabbs, int count, float maxDistance)
{
	std::vector<RayCastHit<Vector>> hits;
	hits.reserve(count);

	Line<vec3f> line(ray.origin, ray.origin + (ray.normal * maxDistance));

	for (int i = 0; i < count; i++)
	{
		Vector poi;
		float t;
		if (IntersectLineAABB(ray, aabbs[i], poi, t))
		{
			hits.push_back({ &aabbs[i], poi , 0.0f });	// Temp store zero for distance to save on calculations
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	float minIndex = 0;

	float prevDistance = magnitudeSquared(hits.at(0).hit->origin - ray.origin);
	for (int i = 1; i < hits.size(); i++)
	{
		float distance = magnitudeSquared(hits.at(i).hit->origin - ray.origin);
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
int RayCast(RayCastHit<Vector>* hitInfo, Ray<Vector> ray, Sphere<Vector>* spheres, int count)
{
	uint32_t	minIndex;
	float		tMin = FLT_MAX;

	for (int i = 0; i < count; i++)
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
	hitInfo->hit = spheres[minIndex];
	hitInfo->poi = ray.origin + ray.normal * tMin;
	hitInfo->distance = cliqCity::graphicsMath::magnitude(spheres[minIndex].origin - ray.origin);

	return 1;
}