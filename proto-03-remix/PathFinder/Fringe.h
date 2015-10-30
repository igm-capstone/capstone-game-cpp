#pragma once
#include <list>
#include <unordered_map>
#include "Graph.h"

namespace PathFinder
{
	using namespace std;

	template<class T>
	struct SearchResult
	{
		float pathCost;
		list<T*> path;
	};

	template<class T>
	struct CacheData
	{
		T* parent;
		float cost;
	};

	template<class T, int width, int height>
	class Fringe
	{
	private:
		unordered_map<T*, CacheData<T>> cache;

	public:
		Graph<T, width, height> graph;
		Fringe() {}
		Fringe(Graph<T, width, height> _graph) : graph(_graph) {}
		~Fringe() {}

		SearchResult<T> FindPath(T* startNode, T* endNode)
		{
			auto result = SearchResult<T>();
			auto fringe = list<T*>();
			auto fLimit = graph.Heuristic(startNode, endNode);
			auto found = false;

			cache.clear();

			fringe.push_front(startNode);
			cache[startNode] = CacheData<T>();

			auto itCount = 0;

			while (!found && !fringe.empty())
			{
				auto fMin = FLT_MAX;
				
				for (auto nodeIt = fringe.begin(); nodeIt != fringe.end();)
				{
					if (itCount++ > 1000) return result;

					auto node = *nodeIt;
					auto nodeData = cache[node];
					auto fNode = nodeData.cost + graph.Heuristic(node, endNode);

					if (fNode > fLimit)
					{
						fMin = min(fNode, fMin);
						nodeIt = ++nodeIt;
						continue;
					}

					if (*node == *endNode)
					{
						found = true;
						endNode = node;
						break;
					}

					auto connections = graph.GetNodeConnections(node);
					for(Connection<T> conn : *connections)
					{
						T* connNode = conn.to;
						auto connCost = nodeData.cost + conn.cost * connNode->weight;

						auto connData = cache[connNode];
						if (connData.parent != nullptr && connCost >= connData.cost)
						{
							continue;
						}

						auto linkedConn = find(fringe.begin(), fringe.end(), connNode);
						if (linkedConn != fringe.end())
						{
							fringe.remove(connNode);
						}
						
						auto insertIt = nodeIt;
						++insertIt;
						fringe.insert(insertIt, connNode);

						connData.parent = node;
						connData.cost = connCost;

						// do i need to reassign?
						cache[connNode] = connData;
					}

					++nodeIt;
					fringe.remove(node);
				}

				fLimit = fMin;
			}

			if (!found)
			{
				return result;
			}

			result.pathCost = cache[endNode].cost;

			auto pathNode = endNode;
			while (pathNode != nullptr)
			{
				result.path.push_back(pathNode);
				pathNode = cache[pathNode].parent;
			}


			return result;
		}
	};
}
