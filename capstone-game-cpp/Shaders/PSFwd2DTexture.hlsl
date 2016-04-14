struct Pixel
{
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD;
	float2 midUV : TEXCOORD1;
	float2 maxUV : TEXCOORD2;
	float maxAngle : TEXCOORD3;
	float3 tint : COLOR;
	float sdf : TEXCOORD4;
};

Texture2DArray diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

static const float PI = 3.14159265f;

float4 main(Pixel pixel) : SV_TARGET
{
	float4 s = diffuseTexture.Sample(samplerState, pixel.uv);

	//SDF
	float smoothing = 0.25f / (4 * pixel.sdf);
	s.a = (pixel.sdf != -1.0f) * smoothstep(0.5 - smoothing, 0.5 + smoothing, s.a) + (pixel.sdf == -1.0f) * s.a;

	// Linear fill
	float alpha = (pixel.uv.x <= pixel.maxUV.x && pixel.uv.y <= pixel.maxUV.y);
	s.a *= alpha;

	// Radial fill
	float2 dir = pixel.midUV - pixel.uv.xy;
	float2 ref = float2(0.0f, 1.0f);
	float angle = acos(dot(dir, ref) / (length(dir)));
	angle = (2 * PI - angle) * (float)(pixel.uv.x < pixel.midUV.x) + angle * (float)(pixel.uv.x >= pixel.midUV.x); //Solves the acos ambiguity without a if-clause

	alpha = (pixel.maxAngle == -1 || angle < pixel.maxAngle);
	s.a *= alpha;

	// Tint
	s *= float4(pixel.tint, 1);


	return s;
}