#pragma once

template<class Vector>
struct Ray
{
	Vector origin;
	Vector direction;
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
	Vector halfSize;
};

template<class Vector>
struct Sphere
{
	Vector origin;
	float radius;
}; 