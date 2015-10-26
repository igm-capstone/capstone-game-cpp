struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float4 mColor		: COLOR;
};

struct PS_OUT
{
	float4  color	: SV_TARGET0;
	float4  shadow	: SV_TARGET1;
};

PS_OUT main(Pixel pixel) : SV_TARGET
{
	PS_OUT output;
	output.color = pixel.mColor;
	output.shadow = 0;
	return output;
}