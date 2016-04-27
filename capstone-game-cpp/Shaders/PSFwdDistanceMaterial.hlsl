struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};

cbuffer PointLight : register(b0)
{
	float4	lightColor;
	float3	lightPosition;
	float	lightRadius;
}

float4 main(Pixel pixel) : SV_TARGET
{
	float3 distance = lightPosition - pixel.positionT;
	return float4(length(distance), 0.0f, 0.0f, 1.0f);
}