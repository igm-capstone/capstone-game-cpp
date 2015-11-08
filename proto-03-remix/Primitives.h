#pragma once

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
	Vector radius;
};

template<class Vector>
struct Sphere
{
	Vector origin;
	float radius;
}; 

typedef AABB<vec2f>		BoxCollider2D;
typedef Sphere<vec2f>	CirceCollider;
typedef AABB<vec3f>		BoxCollider;
typedef Sphere<vec3f>	SphereCollider;