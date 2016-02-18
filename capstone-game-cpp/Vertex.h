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

struct Sprite
{
	vec3f pointpos;
	vec2f size;
	vec2f scale;
	float id;
};

struct SkinnedVertex
{
	uint32_t	BlendIndices[4];
	float		BlendWeights[4];
	vec3f		Position;
	vec3f		Normal;
	vec3f		UV;
};