struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float3 uv			: TEXCOORD;
};

struct PS_OUT
{
	float4 position : SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 color	: SV_TARGET2;
};

Texture2DArray textureArray : register(t0);
SamplerState samplerState : register(s0);

PS_OUT main(Pixel pixel)
{
	PS_OUT ps_out;

	ps_out.position = float4(pixel.positionT, 1.0f);
	ps_out.normal = float4(normalize(pixel.normal), 1.0f);
	ps_out.color = textureArray.Sample(samplerState, pixel.uv, 0);

	return ps_out;
}