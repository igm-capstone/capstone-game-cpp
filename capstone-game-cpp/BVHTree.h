#pragma once
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>

typedef int(*IntersectionTest)(BaseColliderComponent*, OrientedBoxColliderComponent*);

struct BVHNode
{
	class BaseColliderComponent*	object;
	short							parentIndex;

	BVHNode() : object(nullptr), parentIndex(0)
	{
		
	}
	
	~BVHNode()
	{
		object = nullptr;
	}
};

class BVHTree : public BaseSceneObject
{
public:
	std::vector<BVHNode> mNodes;

	vec3f mOrigin;
	vec3f mExtents;

	BVHTree();
	~BVHTree();

	void SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount);
	void Initialize();
	void Update();

	void BuildBoundingVolumeHierarchy();

	void AddNode(BaseColliderComponent* pColliderComponent, const int& parentIndex, const int& depth);
	void AddNodeRecursively(class BaseColliderComponent* pColliderComponent, const int& destLayerIndex, const int& layerIndex, const int& parentIndex, const int& depth, IntersectionTest intersectionTest);

	void RenderDebug();
};
