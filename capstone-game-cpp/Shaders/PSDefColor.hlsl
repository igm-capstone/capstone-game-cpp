struct Pixel
{
	float4 positionH	: SV_POSITION;
	float3 positionT	: POSITIONT;
	float3 normal		: NORMAL;
	float4 color		: COLOR;
};

struct PS_OUT
{
	float4 position : SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 color	: SV_TARGET2;
};

PS_OUT main(Pixel pixel)
{
	PS_OUT ps_out;

	ps_out.position = float4(pixel.positionT, 1.0f);
	ps_out.normal = float4(normalize(pixel.normal), 1.0f);
	ps_out.color = pixel.color;

	return ps_out;
}