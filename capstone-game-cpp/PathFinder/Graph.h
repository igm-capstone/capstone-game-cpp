#pragma once

//#include <cstdint>
//#include <cfloat>
#include <vector>
#include "trace.h"
#include <memory>

namespace PathFinder
{
	using namespace std;

	struct INode {
		float weight;
		int16_t x;
		int16_t y;
		
		INode() : weight(1), x(0), y(0) {}
		INode(int16_t _x, int16_t _y) : weight(1), x(_x), y(_y) {}
		INode(int16_t _x, int16_t _y, float _weight) : weight(_weight), x(_x), y(_y) {}

		bool operator==(const INode& rhs) const
		{
			return x == rhs.x && y == rhs.y && weight == rhs.weight;
		}
	};

	template<class T>
	struct Connection
	{
		float cost;
		T* from;
		T* to;

		Connection() : cost(1), from(nullptr), to(nullptr) {}
		Connection(float _cost, T* _from, T* _to) : cost(_cost), from(_from), to(_to) {}
	};


	template<class T, int width, int height>
	class Graph
	{
	private:

	public:
		T grid[width][height] = {};
		Graph()
		{
			TRACE("Hello " << 1);

			for (auto i = 0; i < width; i++)
			{
				for (auto j = 0; j < height; j++)
				{
					grid[i][j].x = i;
					grid[i][j].y = j;
				}
			}
		}

		~Graph()
		{

		}

		float Heuristic(T* from, T* to) const
		{
			auto dx = abs(from->x - to->x);
			auto dy = abs(from->y - to->y);
			//return dx + dy;
			// diagonal
			auto d = fmin(dx, dy);

			// Manhattan distance on a square grid
			return float(abs(dy - dx) + (d * 1.4f));
		}

		vector<Connection<T>>* GetNodeConnections(T* node)
		{
			bool diagonal = true;
			auto x = node->x;
			auto y = node->y;

			auto notLeftEdge = x > 0;
			auto notRightEdge = x < width - 1;
			auto notBottomEdge = y > 0;
			auto notTopEdge = y < height - 1;
			auto connections = new vector<Connection<T>>();
			connections->reserve(8);

			if (notTopEdge) CreateConnectionIfValid(connections, node, &grid[x][y + 1]);
			if (diagonal && notRightEdge && notTopEdge) CreateConnectionIfValid(connections, node, &grid[x + 1][y + 1]);
			if (notRightEdge) CreateConnectionIfValid(connections, node, &grid[x + 1][y]);
			if (diagonal && notRightEdge && notBottomEdge) CreateConnectionIfValid(connections, node, &grid[x + 1][y - 1]);
			if (notBottomEdge) CreateConnectionIfValid(connections, node, &grid[x][y - 1]);
			if (diagonal && notLeftEdge && notBottomEdge) CreateConnectionIfValid(connections, node, &grid[x - 1][y - 1]);
			if (notLeftEdge) CreateConnectionIfValid(connections, node, &grid[x - 1][y]);
			if (diagonal && notLeftEdge && notTopEdge) CreateConnectionIfValid(connections, node, &grid[x - 1][y + 1]);

			return connections;
		}

		void CreateConnectionIfValid(vector<Connection<T>>* list, T* nodeFrom, T* nodeTo)
		{
			if (nodeTo->weight < FLT_MAX)
			{
				// 1.4 for diagonals and 1 for horizontal or vertical connections
				auto cost = (nodeFrom->x == nodeTo->x || nodeFrom->y == nodeTo->y) ? 1 : 1.4f;
				auto conn = Connection<T>(cost, nodeFrom, nodeTo);
				
				list->push_back(conn);
			}
		}

	};
}

