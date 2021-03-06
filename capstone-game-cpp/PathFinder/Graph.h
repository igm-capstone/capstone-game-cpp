#pragma once
#include <vector>
#include <memory>

namespace PathFinder
{
	using namespace std;

	struct INode {
		float weight;
		int x;
		int y;
		
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


	template<class T>
	class Graph
	{
	public:
		T* pList = nullptr;
		int mNumCols, mNumRows;
		Graph()	{}
		~Graph() {}
		
		void SetSize(int numCols, int numRows, LinearAllocator* allocator)
		{
			mNumCols = numCols;
			mNumRows = numRows;
			pList = reinterpret_cast<T*>(allocator->Allocate(sizeof(T) * numCols * numRows, alignof(T), 0));
		}

		int Count() { return mNumCols * mNumRows; }

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
			auto i = node->x;
			auto j = node->y;

			auto notLeftEdge = i > 0;
			auto notRightEdge = i < mNumRows - 1;
			auto notBottomEdge = j > 0;
			auto notTopEdge = j < mNumCols - 1;
			auto connections = new vector<Connection<T>>();
			connections->reserve(8);



			if (notTopEdge) CreateConnectionIfValid(connections, node, &(*this)(i, j + 1));
			if (diagonal && notRightEdge && notTopEdge) CreateConnectionIfValid(connections, node, &(*this)(i + 1, j + 1));
			if (notRightEdge) CreateConnectionIfValid(connections, node, &(*this)(i + 1, j));
			if (diagonal && notRightEdge && notBottomEdge) CreateConnectionIfValid(connections, node, &(*this)(i + 1, j - 1));
			if (notBottomEdge) CreateConnectionIfValid(connections, node, &(*this)(i, j - 1));
			if (diagonal && notLeftEdge && notBottomEdge) CreateConnectionIfValid(connections, node, &(*this)(i - 1, j - 1));
			if (notLeftEdge) CreateConnectionIfValid(connections, node, &(*this)(i - 1, j));
			if (diagonal && notLeftEdge && notTopEdge) CreateConnectionIfValid(connections, node, &(*this)(i - 1, j + 1));

			return connections;
		}

		Connection<T> GetBestFitConnection(T* node)
		{
			Connection<T> result;
			result.to = nullptr;

			auto connections = GetNodeConnections(node);
			for (auto& connection : *connections)
			{
				if (connection.to->weight >= 0 && (result.to == nullptr || connection.to->weight < result.to->weight))
				{
					result = connection;
				}
			}

			delete connections;

			return result;
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

		T& operator ()(int i, int j)
		{
			return pList[i*mNumCols + j];
		}
	};
}

