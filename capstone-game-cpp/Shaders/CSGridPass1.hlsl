struct Node
{
	float weight;
	int x;
	int y;
	bool hasLight;
	float3 worldPos;
};

struct SimpleNode
{
	float weight;
	int hasLight;
};

cbuffer camera : register(b0)
{
	matrix projection;
	matrix view;
}

cbuffer gridData : register(b1)
{
	int gridNumCols;
	int gridNumRows;
	int mapTexWidth;
	int mapTexHeight;
}

StructuredBuffer<Node> BufferIn : register(t0);
RWStructuredBuffer<SimpleNode> BufferOut : register(u0);

Texture2D Shadows : register(t1);
Texture2D Obstacles : register(t2);

[numthreads(6, 6, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	int x = id.x;
	int y = id.y;
	int flatID = x * gridNumCols + y;
	float nodeRadius = 0.4f;

	Node i = BufferIn[flatID];
	SimpleNode o;
	o.weight = i.weight;
	o.hasLight = i.hasLight;

	if (i.weight == -10) {

		matrix clip = mul(view, projection);

		bool isLight = false;
		bool isObstacle = false;
		
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {

				float4 screenPos = mul(float4(i.worldPos + float3(x*nodeRadius, y*nodeRadius, 0), 1.0f), clip);
				screenPos = screenPos / screenPos.w;
				screenPos.x = round(((screenPos.x + 1) / 2.0f) * mapTexWidth);
				screenPos.y = round(((1 - screenPos.y) / 2.0f) * mapTexHeight);

				//Sample shadow map
				float4 shadow = Shadows.Load(int3(screenPos.x, screenPos.y, 0));
				float4 obstacle = Obstacles.Load(int3(screenPos.x, screenPos.y, 0));
				
				isLight = isLight | ((shadow.r + shadow.g + shadow.b) != 0);
				isObstacle = isObstacle | (obstacle.a != 0);
			}
		}
		o.hasLight = isLight;
		o.weight = isObstacle ? -2 : -1;
	}
 
	BufferOut[flatID] = o;
}