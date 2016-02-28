struct Pixel
{
	float4	positionH	:	SV_POSITION;
	float3	lightPos	:	POSITIONT;
	float2	uv			:	TEXCOORD;
};

Texture2D positionMap	: register(t0);
Texture2D normalMap		: register(t1);
Texture2D depthMap		: register(t2);

SamplerState depthSamplerState	: register(s0);

cbuffer light : register (b0)
{
	matrix	lightViewProjection;
	float4	lightColor;
	float3  lightDirection;
	float	cosAngle;
	float	range;
}

cbuffer camera : register(b1)
{
	float4 eyePos;
}

float4 main(Pixel pixel) : SV_TARGET
{
	int3 i = int3(pixel.positionH.xy, 0);

	float3 position = positionMap.Load(i).xyz;
	float3 normal = normalize(normalMap.Load(i).xyz);

	matrix T =
	{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};

	// Compute transform for homogenous light space
	matrix VPT = mul(lightViewProjection, T);

	// Transform world position to homogenous light space
	float4 lightPosH = mul(float4(position, 1.0f), VPT);

	float bias = 0.0001f;
	float2 projectedUV;
	
	// Compute texture coordinates
	projectedUV.x = lightPosH.x / lightPosH.w;
	projectedUV.y = lightPosH.y / lightPosH.w;

	float depth = depthMap.Sample(depthSamplerState, projectedUV).r;
	float lightDepthValue = (lightPosH.z / lightPosH.w) - bias;

	if (lightDepthValue > depth)
	{
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 diffuse = lightColor;
	float4 specular = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float3 pixelToLight = pixel.lightPos - position;
	float d = length(pixelToLight);
	pixelToLight /= d;

	float nDotL = dot(pixelToLight, normal);
	diffuse *= saturate(nDotL);

	if (nDotL > 0.0f)
	{
		float3 v = reflect(pixelToLight, normal);
		float3 pixelToEye = normalize(eyePos.xyz - position);
		float specFactor = pow(max(dot(v, pixelToEye), 0.0f), 32.0f);

		diffuse += specular * specFactor;
	}

	float4 albedo = diffuse  ;
			
	float	cutoff = cosAngle;	// Input as cos(half spotlight cone angle)
	float	cosAlpha = dot(pixelToLight, -normalize(lightDirection));
	float totalAttenuation = 0.0f;
	if (cosAlpha > cutoff) {
		totalAttenuation = (1.0f - (1.0f - cosAlpha) * 1.0f / (1.0f - cutoff));
	}

	return albedo * totalAttenuation;
}