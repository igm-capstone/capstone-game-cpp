struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
};
//
//struct PS_OUT
//{
//	float4 position : SV_TARGET0;
//	float4 normal	: SV_TARGET1;
//	float4 color	: SV_TARGET2;
//};

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState samplerState : register(s0);

cbuffer time : register (b0)
{
	float	deltaTime;
	float	rate;
	float2	padding;
}

float4 main(Pixel pixel) : SV_TARGET
{
	//PS_OUT ps_out;

	//ps_out.position = float4(pixel.positionT, 1.0f);
	//ps_out.normal = float4(normalize(pixel.normal), 1.0f);
	//ps_out.color = diffuseTexture.Sample(samplerState, pixel.uv + rate * deltaTime);

	return float4(1.0f, 1.0f, 1.0f, 0.5f);
}