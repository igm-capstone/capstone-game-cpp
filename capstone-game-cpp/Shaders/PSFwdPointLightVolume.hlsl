struct Pixel
{
	float4	positionH	:	SV_POSITION;
	float3	lightPos	:	POSITIONT;
	float2	uv			:	TEXCOORD;
};

Texture2D	positionMap		: register(t0);
Texture2D	normalMap		: register(t1);
TextureCube cubeDepthMap	: register(t2);

SamplerState depthSamplerState	: register(s0);

cbuffer light : register (b0)
{
	matrix	lightViewProjection;
	float4	lightColor;
	float3  lightDirection;
	float	cosAngle;
	float	range;
}

float4 main(Pixel pixel) : SV_TARGET
{
	int3 i = int3(pixel.positionH.xy, 0);

	float3 position = positionMap.Load(i).xyz;
	float3 normal	= normalize(normalMap.Load(i).xyz);

	float3 pixelToLight = pixel.lightPos - position;

	float pixelToLightDistance = length(pixelToLight);

	pixelToLight /= pixelToLightDistance;

	float cubeMapDistance = cubeDepthMap.Sample(depthSamplerState, -pixelToLight).r;
	
	float bias = 0.001f;

	if (pixelToLightDistance - bias > cubeMapDistance )
	{
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	int d = (range > pixelToLightDistance);
	// This is for debugging the stencil pass
	//if (!d)
	//{
	//	return float4(0, 0, 1, 1);
	//}

	float4 diffuse = lightColor;
	float nDotL = dot(pixelToLight, normal);
	return diffuse * saturate(nDotL) * d;
}