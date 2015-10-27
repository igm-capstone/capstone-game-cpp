#pragma once
#include "Intersection.h"
#include <vector>

using namespace cliqCity::graphicsMath;

template<class Vector>
struct RayCastHit
{
	AABB<Vector>* hit;
	Vector poi;
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
			hits.push_back({ &aabbs[i], poi });
		}
	}

	if (hits.empty())
	{
		return 0;
	}

	float minIndex = 0;
	float prevDistance = (hits.at(0).hit->origin - ray.origin).magnitude2();
	for (int i = 1; i < hits.size(); i++)
	{
		float distance = (hits.at(i).hit->origin - ray.origin).magnitude2();
		if (distance < prevDistance)
		{
			minIndex = i;
			prevDistance = distance;
		}
	}

	*hitInfo = hits.at(minIndex);

	return 1;
}