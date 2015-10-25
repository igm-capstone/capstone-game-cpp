#pragma once
#include "Rig3D/GraphicsMath/cgm.h"

template<class Vector>
struct Ray
{
	Vector origin;
	Vector normal;
};

template<class Vector>
struct Line
{
	Vector origin;
	Vector end;
};

template<class Vector>
struct AABB
{
	Vector origin;
	float radius;
};

template<class Vector>
struct Sphere
{
	Vector origin;
	float radius;
}; 