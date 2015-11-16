struct Pixel
{
	float4 color;
};

struct GridNode
{
	int hasLight;
};

Texture2D Shadows : register(t0);
Texture2D Grid : register(t1);
RWStructuredBuffer<GridNode> BufferOut : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 grid = Grid.Load(int3(dispatchThreadID.x, dispatchThreadID.y, 0));
	float4 shadow = Shadows.Load(int3(dispatchThreadID.x, dispatchThreadID.y, 0));

	if (grid.w > 0.0f) {
		uint outIndex = (grid.x + grid.y * grid.z);
		BufferOut[outIndex].hasLight = (shadow.x == 0) ? 0 : 1;
	}
}