struct Pixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D diffuseTexture : register(t0);

float4 main(Pixel pixel) : SV_TARGET
{
	int3 i = int3(pixel.position.xy, 0);

	return diffuseTexture.Load(i);
}