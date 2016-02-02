struct Vertex
{
	float3 position :	POSITION;
	float2 uv		:	TEXCOORD;
	float4	color	:	COLOR;
	float4x4 world	:	WORLD;
};

struct Pixel
{
	float4 positionH	:	SV_POSITION;
	float4 lightColor	:	COLOR;
	float3 lightPos		:	POSITIONT;
	float2 uv			:	TEXCOORD;
};

cbuffer transform : register (b0)
{
	matrix projection;
	matrix view;
}

Pixel main( Vertex vertex )
{
	matrix clip = mul(mul(vertex.world, view), projection);

	Pixel pixel;
	pixel.positionH = mul(float4(vertex.position, 1.0f), clip);
	pixel.lightColor = vertex.color;
	pixel.lightPos = float3(vertex.world[3][0], vertex.world[3][1], vertex.world[3][2]);
	pixel.uv = float2((pixel.positionH.x / pixel.positionH.w + 1.0f) * 0.5f, (1.0f - pixel.positionH.y / pixel.positionH.w) * 0.5f);

	return pixel;
}