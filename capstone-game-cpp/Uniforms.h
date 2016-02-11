#pragma once
#include <GraphicsMath/cgm.h>

struct CBufferCamera
{
	mat4f projection;
	mat4f view;
};

struct CBufferModel
{
	mat4f world;
};

struct CBufferLightPVM
{
	mat4f projection;
	mat4f view;
	mat4f world;
};

struct CBufferLight
{
	mat4f		viewProjection;
	vec4f		color;
	vec3f		direction;
	float		cosAngle;
	float		range;
	float		padding[3];
};