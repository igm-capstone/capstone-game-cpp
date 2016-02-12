cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

/*
cbuffer GridAtlasBuffer
{
float sliceWidth;
float sliceHeight;
}

cbuffer PerObjectBuffer
{
	matrix worldMatrix;
	uint sliceIndex;
}*/

struct Vertex
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
	float3		pointpos	: POINTPOS;
	float3		size		: SIZEPX;
	float		id			: TEXID;
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
	float sliceWidth = 100 / 1;
	float sliceHeight = 32 / 2;
	float sliceIndex = 0;

	float textureWidth;
	float textureHeight;
	mTexture.GetDimensions(textureWidth, textureHeight);

	uint slicesX = textureWidth / sliceWidth;
	uint slicesY = textureHeight / sliceHeight;
	uint totalSlices = slicesX * slicesY;
	uint vIndex = sliceIndex / slicesX;
	uint hIndex = sliceIndex - (slicesX * vIndex);

	float4x4 translate = { vertex.size.x / vertex.size.z, 0, 0, 0,
							0, vertex.size.y / vertex.size.z, 0, 0,
							0, 0, 1, 0,
							vertex.pointpos.x, vertex.pointpos.y, vertex.pointpos.z, 1 };

	matrix clip = mul(mul(translate, view), projection);
	output.position = mul(float4(vertex.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	output.uv = float2((vertex.uv.x / slicesX) + (((textureWidth / slicesX) * float(hIndex)) / textureWidth), (vertex.uv.y / slicesY) + (((textureHeight / slicesY) * float(vIndex)) / textureHeight));

	return output;
}