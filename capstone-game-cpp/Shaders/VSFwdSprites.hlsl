cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer spriteSheet : register(b1)
{
	float sheetID;
	float sheetWidth;
	float sheetHeight;
	float slicesX;
	float slicesY;
}

struct Sprite
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
	//Per Sprite
	float3		pointpos	: POINTPOS;
	float2		scale		: SCALE;
	float		spriteID	: SPRITEID;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD;
};

Texture2DArray mTexture : register(t0);

Pixel main(Sprite input)
{
	Pixel output;

	float sliceIndex = input.spriteID;

	uint totalSlices = slicesX * slicesY;
	uint vIndex = sliceIndex / slicesX;
	uint hIndex = sliceIndex - (slicesX * vIndex);

	float sizeX = sheetWidth / slicesX;
	float sizeY = sheetHeight / slicesY;

	float4x4 translatePivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 1, 0, 1 };
	float4x4 scale = {
		sizeX / 2 * input.scale.x, 0, 0, 0,
		0, sizeY / 2 * input.scale.y, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
	float4x4 translatePosAndUndoPivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		input.pointpos.x - sizeX / 2 , input.pointpos.y - sizeY / 2, 0, 1 };
	
	float4x4 world = mul(mul(translatePivot,scale), translatePosAndUndoPivot);

	matrix clip = mul(world, projection);
	output.position = mul(float4(input.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	output.uv = float3( (input.uv.x / slicesX) + (((sheetWidth / slicesX) * float(hIndex)) / sheetWidth), 
						(input.uv.y / slicesY) + (((sheetHeight / slicesY) * float(vIndex)) / sheetHeight), 
						sheetID);
	
	return output;
}