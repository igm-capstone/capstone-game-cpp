#pragma once
#include "Intersection.h"
#include <vector>

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
			hits.push_back({ &aabbs[i], poi , 0.0f});	// Temp store zero for distance to save on calculations
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	float minIndex = 0;
	// TODO! create magnitude2 function
	float prevDistance = magnitude(hits.at(0).hit->origin - ray.origin);
	for (int i = 1; i < hits.size(); i++)
	{
		float distance = magnitude(hits.at(i).hit->origin - ray.origin);
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