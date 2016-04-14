cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

struct Sheet {
	uint realWidth;
	uint realHeight;
	uint slicesX;
	uint slicesY;
};

cbuffer spriteSheets : register(b1)
{
	Sheet sheets[10];
}

struct Sprite
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
	//Per Sprite
	float3		pointpos	: POINTPOS;
	float2		size		: SIZE;
	float2		linearFill	: LINFILL;
	float		radialFill	: RADFILL;
	uint		sheetID		: SHEETID;
	uint		spriteID	: SPRITEID;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD;
	float2 midUV : TEXCOORD1;
	float2 maxUV : TEXCOORD2;
	float maxAngle : TEXCOORD3;
	float3 tint : COLOR;
};

Pixel main(Sprite input)
{
	Pixel output;

	Sheet sheet = sheets[input.sheetID];
	float2 sheetSize = float2(sheet.realWidth, sheet.realHeight);
	float2 slices = float2(sheet.slicesX, sheet.slicesY);

	float2 sprSize = sheetSize / slices;

	float4x4 scale = {
		input.size.x/2, 0, 0, 0,
		0, input.size.y/2, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
	float4x4 translate = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		input.pointpos.x, input.pointpos.y, 0, 1 };

	float4x4 world = mul(scale, translate);

	matrix clip = mul(world, projection);
	output.position = mul(float4(input.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	float2 texScale = 1024 / sheetSize;
	float2 adjUV = input.uv / texScale;

	uint spriteID = input.spriteID;
	uint vIndex = spriteID / slices.x;
	uint hIndex = spriteID - (slices.x * vIndex);
	float2 sprIndex = float2((float)hIndex, (float)vIndex);


	output.uv.xy = adjUV / slices + ((sprSize * sprIndex) / sheetSize) / texScale;
	output.uv.z = input.sheetID;

	float2 oneUV = (1 / texScale) / slices + ((sprSize * sprIndex) / sheetSize) / texScale;
	float2 midUV = (0.5f / texScale) / slices + ((sprSize * sprIndex) / sheetSize) / texScale;

	output.midUV = midUV;
	output.maxUV = oneUV - ((1 / texScale) / slices) * (1.0f - input.linearFill);
	output.maxAngle = input.radialFill;
	output.tint = float3(1, 1, 1);
	return output;
}