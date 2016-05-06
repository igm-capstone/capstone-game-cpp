struct Vertex
{
	float3	position	:	POSITION;
};

struct Pixel
{
	float4	positionH	:	SV_POSITION;
	float3	lightPos	:	POSITIONT;
	float2	uv			:	TEXCOORD;
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

Pixel main( Vertex vertex )
{
	matrix clip = mul(mul(world, view), projection);

	float4 vertexPos = float4(vertex.position, 1.0f);

	Pixel pixel;

	// Transform vertex position to homogenous clip space
	pixel.positionH = mul(vertexPos, clip);

	// Get light position from world matrix
	pixel.lightPos = float3(world[3][0], world[3][1], world[3][2]);

	// Convert UV to screen space.
	pixel.uv = float2((pixel.positionH.x / pixel.positionH.w + 1.0f) * 0.5f, (1.0f - pixel.positionH.y / pixel.positionH.w) * 0.5f);
	
	return pixel;
}