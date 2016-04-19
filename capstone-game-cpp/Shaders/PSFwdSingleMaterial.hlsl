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
	float4 lightColor;
	float3 camPos;
	float	deltaTime;
}

cbuffer Data : register(b1)
{
	
}

float4 main(Pixel pixel) : SV_TARGET
{
	//PS_OUT ps_out;

	//ps_out.position = float4(pixel.positionT, 1.0f);
	//ps_out.normal = float4(normalize(pixel.normal), 1.0f);
	//ps_out.color = diffuseTexture.Sample(samplerState, pixel.uv + rate * deltaTime);

	pixel.normal = normalize(pixel.normal);

//	float albedo = saturate(dot(pixel.normal, -lightDir));

	float3 eyeDirection = normalize(pixel.positionT - camPos);
	float3 refraction = refract(eyeDirection, pixel.normal, 0.1f);
	
	pixel.uv.x -= refraction.x * 0.5f;
	pixel.uv.y += refraction.y * 0.5f;

	float4 diffuse = diffuseTexture.Sample(samplerState, pixel.uv);

	return (diffuse * lightColor);

	return float4(0.1f, 0.1f, 0.1f, 1.0f);
}