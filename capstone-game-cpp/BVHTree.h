#pragma once
#include <SceneObjects/BaseSceneObject.h>
#include <Components/ColliderComponent.h>

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
	std::unordered_map<CLayer, uint32_t> mLayerStartIndex;

	vec3f mOrigin;
	vec3f mExtents;
	vec3f mQuadrantExtents;

	BVHTree();
	~BVHTree();

	void SetRootBoundingVolume(vec3f origin, vec3f extents, int nodeCount);
	void Initialize();
	void Update();

	void BuildBoundingVolumeHierarchy();

	void AddNode(BaseColliderComponent* pColliderComponent, const int& parentIndex, const int& depth);
	void AddNodeRecursively(class BaseColliderComponent* pColliderComponent, const int& destLayerIndex, const int& layerIndex, const int& parentIndex, const int& depth);
	
	BaseColliderComponent* RayCast(Ray<vec3f> ray, vec3f& poi);
	void RayCastRecursively(Ray<vec3f> ray, const int& parentLayerIndex, BaseColliderComponent** pCollider, vec3f& poi, float& tMin);

	void GetNodeIndices(std::vector<uint32_t>& indices, const CLayer& layer, std::function<bool(const BVHNode& other)> predicate);
	void GetNodeIndices(std::vector<uint32_t>& indices, std::function<bool(const BVHNode& other)> predicate);
	BVHNode* GetNode(const uint32_t& index);
};
