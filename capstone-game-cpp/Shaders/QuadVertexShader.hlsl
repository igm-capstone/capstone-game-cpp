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
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
};

cbuffer transform : register(b0)
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
	pixel.normal = mul(float4(vertex.normal, 0.0f), vertex.world).xyz;

	return pixel;
}