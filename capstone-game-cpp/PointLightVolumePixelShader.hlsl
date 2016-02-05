struct Pixel
{
	float4 positionH	:	SV_POSITION;
	float4 lightColor	:	COLOR;
	float3 lightPos		:	POSITIONT;
	float2 uv			:	TEXCOORD;
	float  radius : TEXCOORD1;
};

Texture2D positionMap	: register(t0);
Texture2D normalMap		: register(t1);

SamplerState samplerState : register(s0);

float4 main(Pixel pixel) : SV_TARGET
{
	//return normalMap.Sample(samplerState, pixel.uv);

	float3 position = positionMap.Sample(samplerState, pixel.uv).xyz;
	float3 normal = normalize(normalMap.Sample(samplerState, pixel.uv).xyz);
	//float3 lightDirection = pixel.lightPos - position;
	//float  magnitude = length(lightDirection);
	//lightDirection /= magnitude;

	//float3 lightAttenuation = { 0.0f, 1.0f, 1.0f };
	//float attenuation = saturate(1.0f - magnitude / pixel.radius); // 0.1f / dot(lightAttenuation, float3(1.0f, magnitude, magnitude * magnitude));
	//float nDotL = saturate(dot(normal, lightDirection));
	//return nDotL * pixel.lightColor * attenuation;

	float4 diffuse = pixel.lightColor;
	float4 specular = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float3 lightDirection = float3(1.0f, 0.0f, 0.0f);
	float3 pixelToLight = pixel.lightPos - position;
	float d = length(pixelToLight);
	pixelToLight /= d;

	float df = dot(pixelToLight, normal);
	float3 eyePos = float3(10.0f, 0.0f, -100.0f);
	if (df > 0.0f)
	{
		float3 v = reflect(pixelToLight, normal);
		float3 pixelToEye = normalize(eyePos - position);
		float specFactor = pow(max(dot(v, pixelToEye), 0.0f), 32.0f);

		diffuse *= df * diffuse;
		specular *= specFactor;
	}

	float4 returnColor = (diffuse + specular) / (d * d);

	float   phi = 1.57079632679f * 0.5f;
	float	cutoff = cos(phi);
	float	cosAlpha = dot(pixelToLight, -normalize(lightDirection));
	float totalAttenuation = 0.0f;
	if (cosAlpha > cutoff) {
		totalAttenuation = (1.0f - (1.0f - cosAlpha) * 1.0f / (1.0f - cutoff));
	}

	float4 ac = float4(0.3f, 0.3f, 0.3f, 1.0f);

	return (ac + returnColor) * totalAttenuation;
}