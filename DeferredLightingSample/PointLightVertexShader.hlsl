struct Vertex
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
};

struct Pixel
{
	float4 position	: SV_POSITION;
	float4 color	: COLOR;
};

cbuffer transform : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

cbuffer Color : register(b1)
{
	float4 color;
}

Pixel main(Vertex vertex)
{
	matrix clip = mul(mul(world, view), projection);

	Pixel pixel;
	pixel.position = mul(float4(vertex.position, 1.0f), clip);
	pixel.color = color;

	return pixel;
}