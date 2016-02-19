#pragma once


enum BVHLayer : short
{
	BVH_LAYER_ROOT,
	BVH_LAYER_FLOOR,
	BVH_LAYER_WALL,
	BVH_LAYER_EXPLORER,
	BVH_LAYER_MINION,
	BVH_LAYER_SKILL,
	BVH_LAYER_LAMP
};

struct BVHNode
{
	class BaseColliderComponent*	object;
	BVHLayer						layer;
	short							parentIndex;
};

class BVHTree
{
public:
	BVHNode*	mNodes;
	int			mNodeCount;

	vec3f mOrigin;
	vec3f mExtents;

	BVHTree();
	~BVHTree();

	void SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount);
	void Initialize();
	void Update();

	void BuildBoundingVolumeHierarchy();

	void AddNode(class BaseColliderComponent* pSceneObject, BVHLayer layer, const int& index, const int& parentIndex, const int& depth);
};
