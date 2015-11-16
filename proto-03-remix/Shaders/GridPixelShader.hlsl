struct Pixel
{
	float4 mPositionH	: SV_POSITION;
	float4 mCode		: COLOR;
};

float4 main(Pixel pixel) : SV_TARGET
{
	float4 output;

	output.w = 1.0f;
	output.xyz = pixel.mCode.xyz;

	return output;
}