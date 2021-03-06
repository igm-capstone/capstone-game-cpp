#pragma once
#include "Rig3D/GraphicsMath/cgm.h"

namespace GPU {

	struct Vertex1
	{
		vec3f Position;
		void SetPosition(vec3f val) { this->Position = val; }
		
		void SetColor(vec4f) { return; }
		void SetNormal(vec3f) { return; }
		void SetScale(vec3f) { return; }
		void SetUV(vec2f) { return; }
		void SetBlendIndices(int idx, uint32_t val) { return; }
		void SetBlendWeights(int idx, float val) { return; }
	};

	struct Vertex2
	{
		vec3f Position;
		vec4f Color;
		void SetPosition(vec3f val) { this->Position = val; }
		void SetColor(vec4f val) { this->Color = val; }

		void SetNormal(vec3f) { return; }
		void SetScale(vec3f) { return; }
		void SetUV(vec2f) { return; }
		void SetBlendIndices(int idx, uint32_t val) { return; }
		void SetBlendWeights(int idx, float val) { return; }
	};

	struct Vertex3
	{
		vec3f Position;
		vec3f Normal;
		vec2f UV;
		void SetPosition(vec3f val) { this->Position = val; }
		void SetNormal(vec3f val) { this->Normal = val; }
		void SetUV(vec2f val) { this->UV = val; }

		void SetColor(vec4f) { return; }
		void SetScale(vec3f) { return; }
		void SetBlendIndices(int idx, uint32_t val) { return; }
		void SetBlendWeights(int idx, float val) { return; }
	};

	struct VertexUV
	{
		vec3f Position;
		vec2f UV;
		void SetPosition(vec3f val) { this->Position = val; }
		void SetUV(vec3f val) { this->UV = val; }

		void SetColor(vec4f) { return; }
		void SetNormal(vec3f) { return; }
		void SetScale(vec3f) { return; }
		void SetUV(vec2f) { return; }
		void SetBlendIndices(int idx, uint32_t val) { return; }
		void SetBlendWeights(int idx, float val) { return; }
	};

	struct SkinnedVertex
	{
		uint32_t	BlendIndices[4];
		float		BlendWeights[4];
		vec3f		Position;
		vec3f		Normal;
		vec3f		UV;
		void SetPosition(vec3f val) { this->Position = val; }
		void SetNormal(vec3f val) { this->Normal = val; }
		void SetBlendIndices(int idx, uint32_t val) { this->BlendIndices[idx] = val; }
		void SetBlendWeights(int idx, float val) { this->BlendWeights[idx] = val; }

		void SetColor(vec4f) { return; }
		void SetScale(vec3f) { return; }
		void SetUV(vec2f val) { this->UV = val; }
	};

	struct Sprite
	{
		vec3f pointpos;
		vec2f size;
		vec4f tint;
		vec2f linearFill;
		float radialFill;
		int sheetID;
		int spriteID;
	};

	struct Glyph
	{
		vec3f pointpos;
		vec2f size;
		vec2f scale;
		vec4f tint;
		vec2f minUV;
		vec2f maxUV;
		int sheetID;
	};

	struct Node
	{
		float weight;
		int x;
		int y;
		bool hasLight;
		vec3f worldPos;
	};

	struct SimpleNode
	{
		float weight;
		bool hasLight;
	};
}