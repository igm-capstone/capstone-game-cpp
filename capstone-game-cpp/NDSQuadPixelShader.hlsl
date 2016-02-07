struct Pixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

Texture2D diffuseMap : register(t0);
Texture2D lightMap		: register(t1);
Texture2D depthMap		: register(t2);

SamplerState samplerState : register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	int3 i = int3(pixel.position.xy, 0);

	float4 albedo = lightMap.Load(i);
	float4 diffuse = diffuseMap.Load(i);
	int depth = (int)(depthMap.Load(i).r);

	//float4 albedo = lightMap.Sample(samplerState, pixel.uv);
	//float4 diffuse = diffuseMap.Sample(samplerState, pixel.uv);
	//int depth = (int)(depthMap.Sample(samplerState, pixel.uv).r);
	float4 clearColor = { 0.3, 0.3f, 0.3f, 1.0f };
	float4 ambientLightColor = { 0.5, 0.5f, 0.5f, 1.0f };
	return (clearColor * depth) + (ambientLightColor * !depth * diffuse) + (albedo * diffuse);
}