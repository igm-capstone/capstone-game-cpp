struct SimpleNode
{
	float weight;
	int x;
	int y;
	bool hasLight;
};

struct Connection
{
	float cost;
	SimpleNode nodeTo;
};

cbuffer gridData : register(b1)
{
	int gridNumCols;
	int gridNumRows;
	int screenWidth;
	int screenHeight;
}

StructuredBuffer<SimpleNode> BufferIn : register(t0);
RWStructuredBuffer<SimpleNode> BufferOut : register(u0);


SimpleNode getNode(int x, int y) {
	int flatID = x * gridNumCols + y;
	return BufferIn[flatID];
}

bool CreateConnectionIfValid(out Connection conn, SimpleNode nodeFrom, SimpleNode nodeTo)
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

void GetConnections(SimpleNode node, out Connection conns[8], out int count)
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


[numthreads(12, 12, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
	int x = id.x;
	int y = id.y;
	int flatID = x * gridNumCols + y;

	//Get node
	SimpleNode n = getNode(x, y);
		
	if (n.weight == -1) {
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