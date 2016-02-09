#pragma once
#include "Rig3D/GraphicsMath/cgm.h"

struct Vertex1
{
	vec3f Position;
};

struct QuadVertex
{
	vec3f Position;
	vec3f Color;
};

struct Vertex3
{
	vec3f Position;
	vec3f Normal;
	vec2f UV;
};

struct NDSVertex
{
	vec3f Position;
	vec2f UV;
};