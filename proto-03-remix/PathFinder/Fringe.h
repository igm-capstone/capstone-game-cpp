#pragma once
#include <list>
#include <map>
#include "Graph.h"

namespace PathFinder
{
	using namespace std;

	struct SearchResult
	{
		float pathCost;
		list<Node> path;
	};

	struct CacheData
	{
		Node* parent;
		int cost;
	};

	template<int width, int height>
	class Fringe
	{
	private:
		map<Node, CacheData> cache;
		Graph<width, height> graph;

	public:
		Fringe() {}
		~Fringe() {}

		SearchResult FindPath(Node startNode, Node endNode)
		{
			auto result = SearchResult();
			auto fringe = list<Node>();
			auto fLimit = graph.Heuristic(startNode, endNode);
			auto found = false;

			fringe.push_front(startNode);

			int itCount;
			while (!found && fringe.size > 0)
			{
				auto fMin = FLT_MAX;
				
				for (auto nodeIt = fringe.begin(); nodeIt != fringe.end();)
				{
					Node node = *nodeIt;
					auto nodeData = cache[node];
					auto fNode = nodeData.cost + graph.Heuristic(node, endNode);

					if (fNode > fLimit)
					{
						fMin = min(fNode, fMin);
						nodeIt = ++nodeIt;
						continue;
					}

					if (node == endNode)
					{
						found = true;
						break;
					}

					for(Connection conn : graph.GetNodeConnections(node))
					{
						auto connNode = *conn.to;
						auto connCost = node.weight + conn.cost * connNode.weight;

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
						
						fringe.insert(nodeIt, connNode);

						connData.parent = &connNode;
						connData.cost = connCost;

						// do i need to reassign?
						cache[connNode] = connData;
					}

					//auto it = nodeIt;
					nodeIt = ++nodeIt;
					fringe.remove(node);

					if (itCount++ > 1000) return;
				}
			}

			return result;
		}
	};
}
