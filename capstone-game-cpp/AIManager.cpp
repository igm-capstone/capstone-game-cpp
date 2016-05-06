#include "stdafx.h"
#include "AIManager.h"
#include <cmath>
#include "Factory.h"
#include "SceneObjects/Explorer.h"

AIManager::AIManager()
	: mPathFinder(mGrid), mGridIsDirty(true)
{
	Update();
}

void AIManager::InitGrid(float left, float top, float width, float height)
{
	mNumCols = int(ceilf(width / mNodeRadius / 2));
	mNumRows = int(ceilf(height / mNodeRadius / 2));
	mNumCols += mNumCols % GRID_MULT_OF != 0 ? GRID_MULT_OF - mNumCols % GRID_MULT_OF : 0;
	mNumRows += mNumRows % GRID_MULT_OF != 0 ? GRID_MULT_OF - mNumRows % GRID_MULT_OF : 0;
	mTop = top;
	mLeft = left;
	mGrid.SetSize(mNumCols, mNumRows, mAllocator);

	ResetGridData();
}

void AIManager::ResetGridData()
{
	//Populate nodes
	for (int i = 0; i < mNumRows; i++) {
		for (int j = 0; j < mNumCols; j++) {
			vec3f nodePos = vec3f(mLeft + (mNodeRadius * 2) * j + mNodeRadius, mTop - (mNodeRadius * 2) * i + mNodeRadius, 0);
			mGrid(i, j).x = i;
			mGrid(i, j).y = j;
			mGrid(i, j).weight = -10;
			mGrid(i, j).worldPos = vec3f(nodePos, 0);
		}
	}

	for each(auto &e in Factory<Explorer>()) {
		if (!e.mIsDead)
		{
			GetNodeAt(e.mTransform->GetPosition())->weight = 0;
		}
	}
}

Node* AIManager::GetNodeAt(vec3f pos)
{
	float i = abs((pos.y - mNodeRadius - mTop) / (mNodeRadius * 2));
	float j = abs((pos.x - mNodeRadius - mLeft) / (mNodeRadius * 2));

	return &(mGrid(int(round(i)), int(round(j))));
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

void AIManager::SetAllocator(LinearAllocator* allocator)
{
	mAllocator = allocator;
}