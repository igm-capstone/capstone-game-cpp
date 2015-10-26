struct Vertex
{
	float3 position : POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
};

struct Pixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

Pixel main(uint id:SV_VERTEXID)
{
	Pixel pixel;

	pixel.position.x = (float)(id / 2) * 4.0 - 1.0;
	pixel.position.y = (float)(id % 2) * 4.0 - 1.0;
	pixel.position.z = 0.0;
	pixel.position.w = 1.0;

	pixel.uv.x = (float)(id / 2) * 2.0;
	pixel.uv.y = 1.0 - (float)(id % 2) * 2.0;


	return pixel;
}