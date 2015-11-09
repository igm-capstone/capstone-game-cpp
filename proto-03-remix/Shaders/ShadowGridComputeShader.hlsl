struct Point
{
	float4 position;
};

struct Pixel
{
	int colour;
};

StructuredBuffer<Point> Buffer0 : register(t0);
RWStructuredBuffer<Pixel> BufferOut : register(u0);

float3 readPixel(int x, int y)
{
	float3 output;
	uint index = (x + y * 1024);

	output.x = (Buffer0[index].position.x);
	output.y = (Buffer0[index].position.y);
	output.z = (Buffer0[index].position.z);

	return output;
}
/*
void writeToPixel(int x, int y, float3 colour)
{
	uint index = (x + y * 1024);

	int ired = (int)(clamp(colour.r, 0, 1) * 255);
	int igreen = (int)(clamp(colour.g, 0, 1) * 255) << 8;
	int iblue = (int)(clamp(colour.b, 0, 1) * 255) << 16;

	BufferOut[index].colour = ired + igreen + iblue;
}*/

[numthreads(2, 2, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float3 pixel = readPixel(dispatchThreadID.x, dispatchThreadID.y);
	int index = dispatchThreadID.x + dispatchThreadID.y * 52;
	BufferOut[index].colour = pixel.x % 3 == 0? 1 : 2;

	//pixel.rgb = pixel.r * 0.3 + pixel.g * 0.59 + pixel.b * 0.11;
	//writeToPixel(dispatchThreadID.x, dispatchThreadID.y, pixel);
}