cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer model : register (b1) 
{
	matrix world;
}
/*
cbuffer GridAtlasBuffer
{
float sliceWidth;
float sliceHeight;
}*/

cbuffer PerObjectBuffer
{
	matrix worldMatrix;
	uint sliceIndex;
}

struct Vertex
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D mTexture : register(t0);

Pixel main(Vertex vertex)
{
	Pixel output;
	float sliceWidth = 48 / 3;
	float sliceHeight = 64 / 2;

	float textureWidth;
	float textureHeight;
	mTexture.GetDimensions(textureWidth, textureHeight);

	uint slicesX = textureWidth / sliceWidth;
	uint slicesY = textureHeight / sliceHeight;
	uint totalSlices = slicesX * slicesY;
	uint vIndex = sliceIndex / slicesX;
	uint hIndex = sliceIndex - (slicesX * vIndex);

	matrix clip = mul(mul(world, view), projection);
	output.position = mul(float4(vertex.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	output.uv = float2((vertex.uv.x / slicesX) + (((textureWidth / slicesX) * float(hIndex)) / textureWidth), (vertex.uv.y / slicesY) + (((textureHeight / slicesY) * float(vIndex)) / textureHeight));

	return output;
}