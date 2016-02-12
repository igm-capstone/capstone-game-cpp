cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer spriteSheet : register(b1)
{
	float sliceWidth;
	float sliceHeight;
	float unit2px;
}

struct Sprite
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
	//Per Sprite
	float3		pointpos	: POINTPOS;
	float2		size		: SIZE;
	float2		scale		: SCALE;
	float		texid		: TEXID;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D mTexture : register(t0);

Pixel main(Sprite input)
{
	Pixel output;

	float sliceIndex = input.texid;

	float textureWidth;
	float textureHeight;
	mTexture.GetDimensions(textureWidth, textureHeight);

	uint slicesX = textureWidth / sliceWidth;
	uint slicesY = textureHeight / sliceHeight;
	uint totalSlices = slicesX * slicesY;
	uint vIndex = sliceIndex / slicesX;
	uint hIndex = sliceIndex - (slicesX * vIndex);

	float4x4 translatePivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 1, 0, 1 };
	float4x4 scale = {
		input.size.x / unit2px * input.scale.x, 0, 0, 0,
		0, input.size.y / unit2px * input.scale.y, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };
	float4x4 translatePosAndUndoPivot = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		input.pointpos.x - input.size.x / unit2px, input.pointpos.y - input.size.y / unit2px, input.pointpos.z, 1 };
	
	float4x4 world = mul(mul(translatePivot,scale), translatePosAndUndoPivot);

	matrix clip = mul(mul(world, view), 1);
	output.position = mul(float4(input.position.xyz, 1.0f), clip);

	// Modify UV coordinates to grab the appropriate slice.
	output.uv = float2((input.uv.x / slicesX) + (((textureWidth / slicesX) * float(hIndex)) / textureWidth), (input.uv.y / slicesY) + (((textureHeight / slicesY) * float(vIndex)) / textureHeight));
	
	return output;
}