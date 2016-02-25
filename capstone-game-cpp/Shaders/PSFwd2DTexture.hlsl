struct Pixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	return diffuseTexture.Sample(samplerState, pixel.uv);
}