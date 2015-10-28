struct Pixel
{
	float4	position	: SV_POSITION;
	float2 uv			: TEXCOORD;
};

SamplerState	mSampler		: register(s0);
Texture2D		mTexture		: register(t0);

cbuffer pointL : register(cb0)
{
	float4 lightPoint;
}

float4 main(Pixel pixel) : SV_TARGET
{
	float4 oldColor = mTexture.Sample(mSampler, pixel.uv);
	float2 newCoords;
	float4 newColor; 
	float2 centerP = float2((lightPoint.x + 1) / 2, 1-(lightPoint.y + 1) / 2);
	
	float step = 0.01;
	float start = 0;
	for (int i = 1; i <= 30; i++) {
		newCoords = float2(pixel.uv.x + (centerP.x - pixel.uv.x)*(start + step*i), pixel.uv.y + (centerP.y - pixel.uv.y)*(start + step*i));
		newColor = mTexture.Sample(mSampler, newCoords);
		if (newColor.r == 0)
		{
			oldColor = newColor;
		}
	}

	step = 0.03;
	start = 0.4;
	for (int j = 1; j <= 20; j++) {
		newCoords = float2(pixel.uv.x + (centerP.x - pixel.uv.x)*(start + step*j), pixel.uv.y + (centerP.y - pixel.uv.y)*(start + step*j));
		newColor = mTexture.Sample(mSampler, newCoords);
		if (newColor.r == 0)
		{
			oldColor = newColor;
		}
	}


	return oldColor;
}