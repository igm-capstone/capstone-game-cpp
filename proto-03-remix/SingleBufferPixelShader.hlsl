struct Pixel
{
	float4	position	: SV_POSITION;
	float2 uv			: TEXCOORD;
};

SamplerState	mSampler		: register(s0);
Texture2D		mTexture		: register(t0);

float4 main(Pixel pixel) : SV_TARGET
{
	float4 oldColor = mTexture.Sample(mSampler, pixel.uv);

	for (int i = 1; i < 25; i++) {
		float2 newCoords = float2(pixel.uv.x + (.5 - pixel.uv.x)*(.1*i), pixel.uv.y + (.5 - pixel.uv.y)*(.1*i));
		float4 newColor = mTexture.Sample(mSampler, newCoords);
		if (newColor.r == 0)
		{
			oldColor = newColor;
		}
	}
	/*newCoords = float2(pixel.uv.x + (.5 - pixel.uv.x)*.85, pixel.uv.y + (.5 - pixel.uv.y)*.85);
	newColor = mTexture.Sample(mSampler, newCoords);
	if (newColor.r == 0)
	{
		oldColor = newColor;
	}*/

	return oldColor;
}