struct GridNode
{
	float weight;
	int x;
	int y;
	float3 worldPos;
	bool hasLight;
};

struct Connection
{
	float cost;
	GridNode nodeTo;
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

StructuredBuffer<GridNode> BufferIn : register(t0);
RWStructuredBuffer<GridNode> BufferOut : register(u0);

Texture2D Shadows : register(t1);
Texture2D Obstacles : register(t2);


GridNode getNode(int x, int y) {
	int flatID = x * gridNumCols + y;
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
	bool diagonal = true;
	int x = node.x;
	int y = node.y;

	bool notLeftEdge = x > 0;
	bool notRightEdge = x < gridNumRows - 1;
	bool notBottomEdge = y > 0;
	bool notTopEdge = y < gridNumCols - 1;

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


[numthreads(10, 10, 1)] 
void main(uint3 id : SV_DispatchThreadID)
{
	int x = id.x;
	int y = id.y;
	int flatID = x * gridNumCols + y;
	//GridNode n = getNode(0, 0);

	
	//Get node
	GridNode n = getNode(x, y);

	if (n.weight == -10) {
		//Get node screenPos
		matrix clip = mul(view, projection);
		float4 screenPos = mul(float4(n.worldPos, 1.0f), clip);
		screenPos = screenPos / screenPos.w;
		screenPos.x = round(((screenPos.x + 1) / 2.0f) * screenWidth);
		screenPos.y = round(((1 - screenPos.y) / 2.0f) * screenHeight);

		//Sample shadow map
		float4 color = Shadows.Load(int3(screenPos.x, screenPos.y, 0));
		bool isShadow = (color.r+color.g+color.b) == 0;
		//bool isObstacle = Obstacles.Load(int3(screenPos.x, screenPos.y, 0)).x == 0;

		//Update and output
		n.hasLight = !isShadow;
		n.weight =/* isObstacle ? -2 :*/ -1;
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