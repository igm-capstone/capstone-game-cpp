struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float4 mColor		: COLOR;
};

float4 main(Pixel pixel) : SV_TARGET
{
	return pixel.mColor;
}