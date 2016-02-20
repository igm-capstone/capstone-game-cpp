#pragma once
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>

struct BVHNode
{
	class BaseColliderComponent*	object;
	short							parentIndex;
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

	void AddNodes(OrientedBoxCollider* boundingVolume, const int& layerIndex, const int& index, const int& parentIndex, const int& depth);
	void AddNode(class BaseColliderComponent* pSceneObject, const int& index, const int& parentIndex, const int& depth);
};
