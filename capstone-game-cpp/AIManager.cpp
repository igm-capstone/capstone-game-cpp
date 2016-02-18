#include "stdafx.h"
#include "AIManager.h"
#include <cmath>

AIManager::AIManager()
	: mPathFinder(mGrid)
{
	Update();
}

void AIManager::InitGrid(float left, float top, float width, float height, LinearAllocator& allocator)
{
	mNumNodesX = int(ceilf(width / mNodeRadius));
	mNumNodesY = int(ceilf(height / mNodeRadius));
	mTop = top;
	mLeft = left;
	mGrid.SetSize(mNumNodesX, mNumNodesY, allocator);

	ResetGridData();
}

void AIManager::ResetGridData()
{
	//Populate nodes
	for (int x = 0; x < mNumNodesX; x++) {
		for (int y = 0; y < mNumNodesY; y++) {
			vec3f nodePos = vec3f(mLeft + (mNodeRadius * 2) * x + mNodeRadius, mTop + (mNodeRadius * 2) * y + mNodeRadius, 0);
			mGrid(x, y).x = x;
			mGrid(x, y).y = y;
			mGrid(x, y).weight = -10;
			mGrid(x, y).worldPos = vec3f(nodePos, 0);
		}
	}
}

Node* AIManager::GetNodeAt(vec3f pos)
{
	float x = (pos.x - mNodeRadius - mLeft) / (mNodeRadius * 2);
	float y = (pos.y - mNodeRadius - mTop) / (mNodeRadius * 2);

	return &(mGrid((int)round(x), (int)round(y)));
}

void AIManager::Update() {


}

SearchResult<Node> AIManager::GetPath(vec3f from, vec3f to) {

	// we want the path backward because we want the
	// pathfinder to retain cache for multiple queries 
	// towards the same target (robots chasing player).
	auto end = GetNodeAt(from);
	auto start = GetNodeAt(to);

	auto result = mPathFinder.FindPath(start, end);

	return result;
}

AIManager::~AIManager()
{
}
