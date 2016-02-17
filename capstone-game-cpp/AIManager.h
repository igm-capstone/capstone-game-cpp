#pragma once
#include "PathFinder\Fringe.h"
#include "GraphicsMath\Vector.hpp"
#include "Node.h"

using namespace PathFinder;

#pragma once
class AIManager
{
public:
	LinearAllocator	mAIAllocator;

	Graph<Node> mGrid;
	Fringe<Node> mPathFinder;

	void InitGrid(float left, float top, float width, float height, LinearAllocator& allocator);
	void Update();

	Node* GetNodeAt(vec3f pos);
	SearchResult<Node> GetPath(vec3f from, vec3f to);

	AIManager();
	AIManager(AIManager const&) = delete;
	void operator=(AIManager const&) = delete;
	~AIManager();

private:
	float mTop;
	float mLeft;
	int mNumNodesX;
	int mNumNodesY;
	const float mNodeRadius = 1.0f;
};

