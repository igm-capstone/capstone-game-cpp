#pragma once
#include <GraphicsMath/cgm.h>

namespace CBuffer {
	struct Camera
	{
		mat4f projection;
		mat4f view;
	};

	struct Model
	{
		mat4f world;
	};

	struct LightPVM
	{
		mat4f projection;
		mat4f view;
		mat4f world;
	};

	struct Light
	{
		mat4f		viewProjection;
		vec4f		color;
		vec3f		direction;
		float		cosAngle;
		float		range;
		float		padding[3];
	};

	struct SpriteSheet
	{
		int sheetWidth;
		int sheetHeight;
		int slicesX;
		int slicesY;
	};

	struct GridData
	{
		int gridNumCols;
		int gridNumRows;
		int mapTexWidth;
		int maxTexHeight;
	};

	struct ObjectType
	{
		vec4f color;
	};

	struct Time
	{
		float delta = 0.0f;
		float rate	= 1.0f;
		float padding[2];
	};
}