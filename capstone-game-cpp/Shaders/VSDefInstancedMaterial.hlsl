struct Vertex
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
	float4x4	world		: WORLD;
	uint		matId		: BLENDINDICES;		// Not used for anim just need a semantic to store a uint
};

struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float3 uv			: TEXCOORD;
};

cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(vertex.world, view), projection);

	float4 vertexPos = float4(vertex.position, 1.0f);

	Pixel pixel;
	pixel.positionH = mul(vertexPos, clip);
	pixel.positionT = mul(vertexPos, vertex.world).xyz;
	pixel.normal = mul(vertex.normal, (float3x3)vertex.world).xyz;
	pixel.uv = float3(vertex.uv, 0.0f + vertex.matId);

	return pixel;
}