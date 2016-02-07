struct Pixel
{
	float4	positionH	:	SV_POSITION;
	float4	lightPosH	:	POSITION;
	float4	lightColor	:	COLOR;
	float3	lightPos	:	POSITIONT;
	float2	uv			:	TEXCOORD;
	float	radius		:	TEXCOORD1;
	float	cosAngle	:	TEXCOORD2;
};

Texture2D positionMap	: register(t0);
Texture2D normalMap		: register(t1);
Texture2D depthMap		: register(t2);

SamplerState samplerState		: register(s0);
SamplerState depthSamplerState	: register(s1);

float4 main(Pixel pixel) : SV_TARGET
{
	int3 i = int3(pixel.positionH.xy, 0);

//	return positionMap.Load(i);

	float3 position = positionMap.Load(i).xyz;
	float3 normal = normalize(normalMap.Load(i).xyz);

	//float3 position = positionMap.Sample(samplerState, pixel.uv).xyz;
	//float3 normal = normalize(normalMap.Sample(samplerState, pixel.uv).xyz);
	//float3 lightDirection = pixel.lightPos - position;
	//float  magnitude = length(lightDirection);
	//lightDirection /= magnitude;

	//float3 lightAttenuation = { 0.0f, 1.0f, 1.0f };
	//float attenuation = saturate(1.0f - magnitude / pixel.radius); // 0.1f / dot(lightAttenuation, float3(1.0f, magnitude, magnitude * magnitude));
	//float nDotL = saturate(dot(normal, lightDirection));
	//return nDotL * pixel.lightColor * attenuation;

	float bias = 0.0001f;
	float2 projectedUV;
	
	projectedUV.x = pixel.lightPosH.x / pixel.lightPosH.w;
	projectedUV.y = pixel.lightPosH.y / pixel.lightPosH.w;

	float depth = depthMap.Sample(depthSamplerState, projectedUV).r;
	float lightDepthValue = (pixel.lightPosH.z / pixel.lightPosH.w) - bias;

	if (lightDepthValue > depth)
	{
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	float4 diffuse = pixel.lightColor;
	float4 specular = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float3 lightDirection = float3(1.0f, 0.0f, 0.0f); // Hard Coded
	float3 pixelToLight = pixel.lightPos - position;
	float d = length(pixelToLight);
	pixelToLight /= d;

	float nDotL = dot(pixelToLight, normal);
	diffuse *= saturate(nDotL);

	float3 eyePos = float3(10.0f, 0.0f, -100.0f);	// Hard Coded.

	if (nDotL > 0.0f)
	{
		float3 v = reflect(pixelToLight, normal);
		float3 pixelToEye = normalize(eyePos - position);
		float specFactor = pow(max(dot(v, pixelToEye), 0.0f), 32.0f);

		diffuse += specular * specFactor;
	}

	float4 albedo = diffuse / (d * 0.5f) ;
			
	float   phi = 1.57079632679f * 0.5f;		// Hard coded
	float	cutoff = cos(phi);
	float	cosAlpha = dot(pixelToLight, -normalize(lightDirection));
	float totalAttenuation = 0.0f;
	if (cosAlpha > cutoff) {
		totalAttenuation = (1.0f - (1.0f - cosAlpha) * 1.0f / (1.0f - cutoff));
	}

	return albedo * totalAttenuation;
}