struct Vertex
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
	float4x4	world		: WORLD;
//	uint		mInstanceID	: SV_InstanceID;
};

struct Pixel
{
	float4 positionH	: SV_POSITION;
};

cbuffer transform : register(b0)
{
	matrix projection;
	matrix view;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(vertex.world, view), projection);

	Pixel pixel;
	pixel.positionH = mul(float4(vertex.position, 1.0f), clip);

	return pixel;
}