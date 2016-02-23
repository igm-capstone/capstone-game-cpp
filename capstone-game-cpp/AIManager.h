#pragma once
#include "PathFinder\Fringe.h"
#include "GraphicsMath\Vector.hpp"
#include "Node.h"

using namespace PathFinder;

#pragma once
class AIManager
{
	LinearAllocator* mAllocator;
public:

	Graph<Node> mGrid;
	Fringe<Node> mPathFinder;

	void InitGrid(float left, float top, float width, float height);
	void ResetGridData();
	void Update() {};

	Node* GetNodeAt(vec3f pos);
	SearchResult<Node> GetPath(vec3f from, vec3f to);

	AIManager();
	AIManager(AIManager const&) = delete;
	void operator=(AIManager const&) = delete;
	~AIManager();
	void SetAllocator(LinearAllocator* allocator);

	void SetGridDirty(bool state) { mGridIsDirty = state; }
	bool IsGridDirty() { return mGridIsDirty; }

private:
	float mTop;
	float mLeft;
	int mNumCols;
	int mNumRows;
	const float mNodeRadius = 0.85f;
	bool mGridIsDirty;
};

