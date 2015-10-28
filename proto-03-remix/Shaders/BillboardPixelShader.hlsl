struct Pixel
{
	float4	position	: SV_POSITION;
	float2 uv			: TEXCOORD;
};

SamplerState	mSampler		: register(s0);
Texture2D		mTexture		: register(t0);

float4 main(Pixel pixel) : SV_TARGET
{
	float4 textureColor = mTexture.Sample(mSampler, pixel.uv);

	return textureColor;
}