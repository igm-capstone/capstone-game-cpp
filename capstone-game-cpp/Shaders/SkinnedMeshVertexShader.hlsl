#define MAX_SKINNED_MATRICES	64
#define MAX_BLEND_MATRICES		4

struct Vertex
{
	uint4	blendIndices	: BLENDINDICES;
	float4	blendWeights	: BLENDWEIGHTS;
	float3  position		: POSITION;
	float3	normal			: NORMAL;
	float2	uv				: TEXCOORD;
};

struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer model : register (b1)
{
	matrix world;
}

cbuffer pose : register(b2)
{
	matrix skinnedMatrices[MAX_SKINNED_MATRICES];
}

Pixel main( Vertex vertex )
{
	float4 vertexPosition = float4(vertex.position, 1.0f);
//	float3 vertexPosePosition = vertex.position;
	float3 vertexPosePosition = float3(0.0f, 0.0f, 0.0f);

	[unroll]
	for (int i = 0; i < MAX_BLEND_MATRICES; i++)
	{
		vertexPosePosition += mul(vertexPosition, skinnedMatrices[vertex.blendIndices[i]]).xyz * vertex.blendWeights[i];
	}

	float4x4 clip = mul(mul(world, view), projection);

	Pixel pixel;
	pixel.positionH = mul(float4(vertexPosePosition, 1.0f), clip);
	pixel.positionT = mul(vertexPosePosition, (float3x3)world);
	pixel.normal	= mul(vertexPosePosition, (float3x3)world);
	pixel.uv		= vertex.uv;

	return pixel;
}