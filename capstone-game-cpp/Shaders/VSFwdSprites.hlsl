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
	float2		scale		: SCALE;
	float2		anchorScale	: ANCHSCALE;
	uint		sheetID		: SHEETID;
	uint		spriteID	: SPRITEID;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD;
};

Pixel main(Sprite input)
{
	Pixel output;

	Sheet sheet = sheets[input.sheetID];
	float sheetWidth = sheet.realWidth;
	float sheetHeight = sheet.realHeight;
	float slicesX = sheet.slicesX;
	float slicesY = sheet.slicesY;

	float sizeX = sheetWidth / slicesX;
	float sizeY = sheetHeight / slicesY;

	float adjU = input.uv.x / (1024 / sheetWidth);
	float adjV = input.uv.y / (1024 / sheetHeight);

	uint spriteID = input.spriteID;

	uint vIndex = spriteID / slicesX;
	uint hIndex = spriteID - (slicesX * vIndex);

	float4x4 translatePivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 1, 0, 1 };
	float4x4 scaleAnchored = {
		sizeX / 2 * input.anchorScale.x, 0, 0, 0,
		0, sizeY / 2 * input.anchorScale.y, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
	float4x4 translateUndoPivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-sizeX / 2 , -sizeY / 2, 0, 1 };

	float4x4 scale = {
		input.scale.x, 0, 0, 0,
		0, input.scale.y, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
	float4x4 translate = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		input.pointpos.x, input.pointpos.y, 0, 1 };

	float4x4 world = mul(mul(mul(mul(translatePivot, scaleAnchored), translateUndoPivot), scale), translate);

	matrix clip = mul(world, projection);
	output.position = mul(float4(input.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	output.uv = float3((adjU / slicesX) + (((sheetWidth / slicesX) * float(hIndex)) / sheetWidth) / (1024 / sheetWidth),
		(adjV / slicesY) + (((sheetHeight / slicesY) * float(vIndex)) / sheetHeight) / (1024 / sheetHeight),
		input.sheetID);

	return output;
}