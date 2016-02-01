struct Pixel
{
	float4 positionH	: SV_POSITION;
};

float4 main(Pixel pixel) : SV_TARGET
{
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}