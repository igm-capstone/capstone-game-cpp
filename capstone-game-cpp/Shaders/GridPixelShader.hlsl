struct Pixel
{
	float4 positionH	: SV_POSITION;
	float4 color		: COLOR;

};

float4 main(Pixel pixel) : SV_TARGET0
{
	return pixel.color;
}