cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

struct Glyph
{
	float4		position	: POSITION;
	float2		uv			: TEXCOORD;
	//Per Sprite
	float3		pointpos	: POINTPOS;
	float2		size		: SIZE;
	float2		scale		: SCALE;
	float3		color		: COLOR;
	float2		minUV		: MINUV;
	float2		maxUV		: MAXUV;
	uint		sheetID		: SHEETID;
};

struct Pixel
{
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD;
	float2 midUV : TEXCOORD1;
	float2 maxUV : TEXCOORD2;
	float maxAngle : TEXCOORD3;
	float3 tint : COLOR;
	float sdf : TEXCOORD4;
};

Pixel main(Glyph input)
{
	Pixel output;
	input.position.xy += float2(0.5f, 0.5f);

	float4x4 scale = {
		input.size.x/2 * input.scale.x, 0, 0, 0,
		0, input.size.y/2 * input.scale.y, 0, 0,
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

	output.uv.xy = (input.maxUV - input.minUV) * input.uv.xy + input.minUV;
	output.uv.z = input.sheetID;

	output.midUV = float2(0,0);
	output.maxAngle = -1;
	output.maxUV = float2(1,1);
	output.tint = input.color;
	output.sdf = input.scale.x;

	return output;
}