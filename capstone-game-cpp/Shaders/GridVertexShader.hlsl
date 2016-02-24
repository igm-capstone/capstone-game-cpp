struct Vertex
{
	float3		position	: POSITION;
	float3		normal		: NORMAL;
	float2		uv			: TEXCOORD;
	float4x4	world		: WORLD;
};

struct Pixel
{
	float4 positionH	: SV_POSITION;
	float4 color		: COLOR;
};

cbuffer transform : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer objectType : register(b1)
{
	float4 color;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(vertex.world, view), projection);

	float4 vertexPos = float4(vertex.position, 1.0f);

	Pixel pixel;
	pixel.positionH = mul(vertexPos, clip);
	pixel.color = color;

	return pixel;
}