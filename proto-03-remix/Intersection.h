#pragma once
#include "Primitives.h"
#include <cmath>
#include <algorithm>
#include <float.h>
#include "Rig3D/GraphicsMath/cgm.h"

template<class Vector>
int RaySphereIntersect(Ray<Vector> ray, Sphere<Vector> sphere, Vector& poi, float& t)
{
	// R(t) = P + tD, where t >= 0
	// S(t) = (X - C) * (X - C) = r^2, where X is a point on the surface of the sphere
	// Substitute R(t) for X to find the value of t for which the ray intersects the sphere

	// Let m = Ray.origin - sphere.origin
	Vector m = ray.origin - sphere.origin;

	// Let b = projection of ray direction onto vector from rayOrigin to sphereOrigin
	float b = cliqCity::graphicsMath::dot(m, ray.normal);							

	// Let c = difference between distance from rayOrigin to sphereOrigin and sphereRadius
	float c = cliqCity::graphicsMath::dot(m, m) - sphere.radius * sphere.radius;

	// If c > 0 rayOrigin is outside of sphere and if b > 0.0f ray is pointing away from sphere.
	if (c > 0.0f && b > 0.0f)
	{
		return 0;
	}

	float discriminant = b * b - c;

	// If discriminant is negative the ray misses the sphere
	if (discriminant < 0.0f)
	{
		return 0;
	}

	t = -b - sqrt(discriminant);
	poi = ray.origin + t * ray.normal;

	return 1;
}

template<class Vector>
int IntersectRayAABB(Ray<Vector> ray, AABB<Vector> aabb, Vector& poi, float& t)
{
	int numElements = sizeof(Vector) / sizeof(float);

	float tMin = 0.0f;
	float tMax = FLT_MAX;

	for (int i = 0; i < numElements; i++)
	{
		float aabbMin = aabb.origin[i] - aabb.radius;
		float aabbMax = aabb.origin[i] + aabb.radius;

		if (abs(ray.direction[i]) < FLT_EPSILON)
		{
			// Ray is parallel to slab. Check if origin is contained by plane
			if (ray.origin[i] < aabbMin || ray.origin[i] > aabbMax)
			{
				return 0;
			}
		}
		else
		{
			float ood = 1.0f / ray.direction[i];
			float t1 = (aabbMin - ray.origin[i]) * ood;
			float t2 = (aabbMax - ray.origin[i]) * ood;

			if (t1 > t2)
			{
				float temp = t2;
				t2 = t1;
				t1 = temp;
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return 0;
			}
		}
	}

	poi = ray.origin + ray.normal * tMin;
	t = tMin;
	return 1;
}

template<class Vector>
int IntersectAABBAABB(AABB<Vector> aabb0, AABB<Vector> aabb1)
{
	int numElements = sizeof(Vector) / sizeof(float);
	
	Vector min0 = aabb0.origin - aabb0.radius;
	Vector max0 = aabb0.origin + aabb0.radius;
	Vector min1 = aabb1.origin - aabb1.radius;
	Vector max1 = aabb1.origin + aabb1.radius;

	for (int i = 0; i < numElements; i++)
	{
		if (max0[i] < min1[i] || min0[i] > max1[i])
		{
			return 0;
		}
	}

	return 1;
}

template<class Vector>
int IntersectSphereAABB(Sphere<Vector> sphere, AABB<Vector> aabb)
{
	int numElements = sizeof(Vector) / sizeof(float);

	Vector aabbMin = aabb.origin - aabb.radius;
	Vector aabbMax = aabb.origin + aabb.radius;
	float s, d = 0.0f;

	for (int i = 0; i < numElements; i++)
	{
		if (sphere.origin[i] < aabbMin[i])
		{
			s = sphere.origin[i] - aabbMin[i];
			d += s * s;
		}
		else if (sphere.origin[i] > aabbMax[i])
		{
			s = sphere.origin[i] - aabbMax[i];
			d += s * s;
		}
	}

	return d <= sphere.radius * sphere.radius;
}