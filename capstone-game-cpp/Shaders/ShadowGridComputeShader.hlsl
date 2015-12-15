struct GridNode
{
	float3 worldPos;
	int x;
	int y;
	float weight;
	bool hasLight;
};

struct Connection
{
	float cost;
	GridNode nodeTo;
};

cbuffer transform : register(b0)
{
	matrix view;
	matrix projection;
}

StructuredBuffer<GridNode> BufferIn : register(t0);
RWStructuredBuffer<GridNode> BufferOut : register(u0);

Texture2D Shadows : register(t1);
Texture2D Obstacles : register(t2);

static const int numThreads = 52;
static const int numGroups = 34;

GridNode getNode(int x, int y) {
	int flatID = x + y * numThreads;
	return BufferIn[flatID];
}

bool CreateConnectionIfValid(out Connection conn, GridNode nodeFrom, GridNode nodeTo)
{
	conn.cost = 0;
	conn.nodeTo = nodeFrom;
	if (nodeTo.weight >= 0)
	{
		// 1.4 for diagonals and 1 for horizontal or vertical connections
		conn.cost = (nodeFrom.x == nodeTo.x || nodeFrom.y == nodeTo.y) ? 1 : 1.4f;
		conn.nodeTo = nodeTo;
		return true;
	}
	return false;
}

void GetConnections(GridNode node, out Connection conns[8], out int count)
{
	int x = node.x;
	int y = node.y;

	bool diagonal = false;
	bool notLeftEdge = x > 0;
	bool notRightEdge = x < numThreads - 1;
	bool notBottomEdge = y > 0;
	bool notTopEdge = y < numGroups - 1;

	Connection c;
	count = 0;

	if (notTopEdge && CreateConnectionIfValid(c, node, getNode(x, y + 1))) {
		conns[count] = c;
		count++;
	}
	if (diagonal && notRightEdge && notTopEdge && CreateConnectionIfValid(c, node, getNode(x + 1, y + 1))) {
		conns[count] = c;
		count++;
	}
	if (notRightEdge && CreateConnectionIfValid(c, node, getNode(x + 1, y))) {
		conns[count] = c;
		count++;
	}
	if (diagonal && notRightEdge && notBottomEdge && CreateConnectionIfValid(c, node, getNode(x + 1, y - 1))) {
		conns[count] = c;
		count++;
	}
	if (notBottomEdge && CreateConnectionIfValid(c, node, getNode(x, y - 1))) {
		conns[count] = c;
		count++;
	}
	if (diagonal && notLeftEdge && notBottomEdge && CreateConnectionIfValid(c, node, getNode(x - 1, y - 1))) {
		conns[count] = c;
		count++;
	}
	if (notLeftEdge && CreateConnectionIfValid(c, node, getNode(x - 1, y))) {
		conns[count] = c;
		count++;
	}
	if (diagonal && notLeftEdge && notTopEdge && CreateConnectionIfValid(c, node, getNode(x - 1, y + 1))) {
		conns[count] = c;
		count++;
	}
}


[numthreads(numThreads, 1, 1)]
void main(uint3 idX : SV_GroupThreadID, uint3 idY : SV_GroupID)
{
	int x = idX.x;
	int y = idY.x;
	int flatID = x + y * numThreads;

	//Get node
	GridNode n = getNode(x, y);

	if (n.weight == -10) {
		//Get node screenPos
		matrix clip = mul(view, projection);
		float4 screenPos = mul(float4(n.worldPos, 1.0f), clip);
		screenPos = screenPos / screenPos.w;
		screenPos.y = -1 * screenPos.y;
		screenPos = screenPos + 1;
		screenPos = screenPos / 2;
		screenPos.x = screenPos.x * 1600;
		screenPos.y = screenPos.y * 1000;
		//Sample shadow map
		bool isShadow = Shadows.Load(int3(screenPos.x, screenPos.y, 0)).x == 0;
		bool isObstacle = Obstacles.Load(int3(screenPos.x, screenPos.y, 0)).x == 0;

		//Update and output
		n.hasLight = !isShadow;
		n.weight = isObstacle ? -2 : -1;
	}
	else if (n.weight == -1) {
		Connection conns[8];
		int count;
		GetConnections(n, conns, count);
		if (count > 0) {
			Connection c = conns[0];
			for (int i = 1; i < count; i++) {
				if (conns[i].nodeTo.weight < c.nodeTo.weight)
					c = conns[i];
			}

			n.weight = c.nodeTo.weight + c.cost;
		}
	}

	BufferOut[flatID] = n;
}