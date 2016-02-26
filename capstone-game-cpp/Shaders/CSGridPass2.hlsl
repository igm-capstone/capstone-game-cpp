struct SimpleNode
{
	float weight;
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
	int mapTexWidth;
	int mapTexHeight;
}

StructuredBuffer<SimpleNode> BufferIn : register(t0);
RWStructuredBuffer<SimpleNode> BufferOut : register(u0);


SimpleNode getNode(int x, int y) {
	int flatID = x * gridNumCols + y;
	return BufferIn[flatID];
}

bool isValidNode (int x, int y) {
	return (x >= 0 && x < gridNumRows &&
			y >= 0 && y < gridNumCols);
}

bool CreateConnectionIfValid(out Connection conn, SimpleNode nodeFrom, float cost, int toX, int toY)
{
	conn.cost = 0;
	conn.nodeTo = nodeFrom;

	if (!isValidNode(toX, toY)) return false;

	SimpleNode nodeTo = getNode(toX, toY);

	if (nodeTo.weight >= 0)
	{
		// 1.4 for diagonals and 1 for horizontal or vertical connections
		conn.cost = cost;
		conn.nodeTo = nodeTo;
		return true;
	}
	return false;
}

void GetConnections(SimpleNode node, int x, int y, out Connection conns[8], out int count)
{
	bool diagonal = true;

	Connection c;
	count = 0;

	if (CreateConnectionIfValid(c, node, 1.0f, x, y + 1)) {
		conns[count] = c;
		count++;
	}
	if (diagonal && CreateConnectionIfValid(c, node, 1.41f, x + 1, y + 1)) {
		conns[count] = c;
		count++;
	}
	if (CreateConnectionIfValid(c, node, 1.0f, x + 1, y)) {
		conns[count] = c;
		count++;
	}
	if (diagonal && CreateConnectionIfValid(c, node, 1.41f, x + 1, y - 1)) {
		conns[count] = c;
		count++;
	}
	if (CreateConnectionIfValid(c, node, 1.0f, x, y - 1)) {
		conns[count] = c;
		count++;
	}
	if (diagonal && CreateConnectionIfValid(c, node, 1.41f, x - 1, y - 1)) {
		conns[count] = c;
		count++;
	}
	if (CreateConnectionIfValid(c, node, 1.0f, x - 1, y)) {
		conns[count] = c;
		count++;
	}
	if (diagonal && CreateConnectionIfValid(c, node, 1.41f, x - 1, y + 1)) {
		conns[count] = c;
		count++;
	}
}


[numthreads(6, 6, 1)]
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
		GetConnections(n, x, y, conns, count);
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