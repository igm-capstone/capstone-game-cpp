struct Vertex
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
};

struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float4 color		: COLOR;
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

cbuffer color : register(b2)
{
	float4 color;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(world, view), projection);

	float4 vertexPos = float4(vertex.position, 1.0f);

	Pixel pixel;
	pixel.positionH = mul(vertexPos, clip);
	pixel.positionT = mul(vertexPos, world).xyz;
	pixel.normal = mul(float4(vertex.normal, 0.0f), world).xyz;
	pixel.color = color;
	return pixel;
}