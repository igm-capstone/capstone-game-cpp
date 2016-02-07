struct Vertex
{
	float3	position	:	POSITION;
	float3	normal		:	NORMAL;
	float2	uv			:	TEXCOORD;
};

struct Pixel
{
	float4	positionH	:	SV_POSITION;
	float4	lightPosH	:	POSITION;
	float4	lightColor	:	COLOR;
	float3	lightPos	:	POSITIONT;
	float2	uv			:	TEXCOORD;
	float	radius		:	TEXCOORD1;
	float	cosAngle	:	TEXCOORD2;
};

cbuffer transform : register (b0)
{
	matrix projection;
	matrix view;
	matrix world;
}

cbuffer light : register (b1)
{
	matrix	lightProjection;
	matrix	lightView;
	float4	lightColor;
	float	cosAngle;
	float	range;
}

Pixel main( Vertex vertex )
{
	matrix clip = mul(mul(world, view), projection);

	float4 vertexPos = float4(vertex.position, 1.0f);

	Pixel pixel;

	// Transform vertex position to homogenous clip space
	pixel.positionH = mul(vertexPos, clip);

	// Transform position to homogenous light space

	matrix T =
	{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};

	matrix WVPT = mul(mul(world, lightView), mul(lightProjection, T));

	// Transform position to homogenous light space
	pixel.lightPosH = mul(vertexPos, WVPT);

	// Pass color
	pixel.lightColor = lightColor;

	// Get light position from world matrix
	pixel.lightPos = float3(world[3][0], world[3][1], world[3][2]);

	// Convert UV to screen space.
	pixel.uv = float2((pixel.positionH.x / pixel.positionH.w + 1.0f) * 0.5f, (1.0f - pixel.positionH.y / pixel.positionH.w) * 0.5f);

	// Get linear range from matrix for now.
	pixel.radius = range;

	// Pass cosAngle
	pixel.cosAngle = cosAngle;
	
	return pixel;
}