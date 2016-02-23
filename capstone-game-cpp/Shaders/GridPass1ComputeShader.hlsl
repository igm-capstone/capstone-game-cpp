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
	bool hasLight;
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
	int screenWidth;
	int screenHeight;
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
	
	Node i = BufferIn[flatID];
	SimpleNode o;
	o.weight = i.weight;
	o.hasLight = i.hasLight;

	if (i.weight == -10) {
		//Get node screenPos
		matrix clip = mul(view, projection);
		float4 screenPos = mul(float4(i.worldPos, 1.0f), clip);
		screenPos = screenPos / screenPos.w;
		screenPos.x = round(((screenPos.x + 1) / 2.0f) * screenWidth);
		screenPos.y = round(((1 - screenPos.y) / 2.0f) * screenHeight);

		//Sample shadow map
		float4 color = Shadows.Load(int3(screenPos.x, screenPos.y, 0));
		bool isShadow = (color.r + color.g + color.b) == 0;
		float4 color2 = Obstacles.Load(int3(screenPos.x, screenPos.y, 0));
		bool isObstacle = color2.a != 0;

		//Update
		o.hasLight = false;
		o.weight = isObstacle ? -2 : -1;
	}
 
	BufferOut[flatID] = o;
}