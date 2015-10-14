#pragma once

#include <cstdint>
#include <cfloat>
#include <vector>
#include "trace.h"

namespace PathFinder
{
	using namespace std;

	struct Node {
		float weight;
		int16_t x;
		int16_t y;
		
		Node() : weight(1), x(0), y(0) {}
		Node(int16_t _x, int16_t _y) : weight(1), x(_x), y(_y) {}
		Node(int16_t _x, int16_t _y, float _weight) : weight(_weight), x(_x), y(_y) {}
	};

	struct Connection
	{
		float cost;
		Node* from;
		Node* to;

		Connection(float _cost, Node* _from, Node* _to) : cost(_cost), from(_from), to(_to) {}
	};


	template<int width, int height>
	class Graph
	{
	private:
		Node grid[width][height] = {};

	public:
		Graph()
		{
			TRACE("Hello " << 1);
		}

		~Graph()
		{

		}

		float Heuristic(Node from, Node to) const
		{
			auto dx = abs(from.x - to.x);
			auto dy = abs(from.y - to.y);

			// diagonal
			auto d = fmin(dx, dy);

			// Manhattan distance on a square grid
			return abs(dx - dy) + (d * 1.4f);
		}

		vector<Connection> GetNodeConnections(Node node)
		{
			auto x = node.x;
			auto y = node.y;

			auto notLeftEdge = x > 0;
			auto notRightEdge = x < width - 1;
			auto notBottomEdge = y > 0;
			auto notTopEdge = y < height - 1;
			auto connections = vector<Connection>(8);

			if (notTopEdge) CreateConnectionIfValid(connections, node, grid[x][ y + 1]);
			if (notRightEdge && notTopEdge) CreateConnectionIfValid(connections, node, grid[x + 1][ y + 1]);
			if (notRightEdge) CreateConnectionIfValid(connections, node, grid[x + 1][ y]);
			if (notRightEdge && notBottomEdge) CreateConnectionIfValid(connections, node, grid[x + 1][ y - 1]);
			if (notBottomEdge) CreateConnectionIfValid(connections, node, grid[x][ y - 1]);
			if (notLeftEdge && notBottomEdge) CreateConnectionIfValid(connections, node, grid[x - 1][ y - 1]);
			if (notLeftEdge) CreateConnectionIfValid(connections, node, grid[x - 1][ y]);
			if (notLeftEdge && notTopEdge) CreateConnectionIfValid(connections, node, grid[x - 1][ y + 1]);

			return connections;
		}

		void CreateConnectionIfValid(vector<Connection> list, Node nodeFrom, Node nodeTo)
		{
			if (nodeTo.weight < FLT_MAX)
			{
				// 1.4 for diagonals and 1 for horizontal or vertical connections
				auto cost = nodeFrom.x == nodeTo.x || nodeFrom.x == nodeTo.x ? 1 : 1.4f;
				auto conn = Connection(cost, &nodeFrom, &nodeTo);
				
				list.push_back(conn);
			}
		}

	};
}

