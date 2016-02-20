struct Node
{
	float weight;
	int x;
	int y;
	float3 worldPos;
	bool hasLight;
};

struct SimpleNode
{
	float weight;
	int x;
	int y;
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

[numthreads(12, 12, 1)] 
void main(uint3 id : SV_DispatchThreadID)
{
	int x = id.x;
	int y = id.y;
	int flatID = x * gridNumCols + y;
	
	Node i = BufferIn[flatID];
	SimpleNode o;
	o.x = i.x;
	o.y = i.y;
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
		//bool isObstacle = Obstacles.Load(int3(screenPos.x, screenPos.y, 0)).x == 0;

		//Update
		o.hasLight = !isShadow;
		o.weight = /* isObstacle ? -2 :*/ -1;
	}
 
	BufferOut[flatID] = o;
}