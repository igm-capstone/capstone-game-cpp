#pragma once
#include <stdint.h>
#include <Rig3D/GraphicsMath/cgm.h>
#include <assert.h>
#include <vector>

struct JointBlendWeight
{
	int		jointIndex;
	float	jointWeight;
};

struct Joint
{
	mat4f		inverseBindPoseMatrix;  // Transforms from model space bind pose to joint space bind pose
	mat4f		animPoseMatrix;			// Transforms from joint space bind pose to model space animated pose
	int			parentIndex;			// -1 indicates root 
	const char* name;
};

struct Keyframe
{
	quatf rotation;
	vec3f scale;
	vec3f translation;
	float time;
};

struct JointAnimation
{
	uint32_t jointIndex;
	std::vector<Keyframe> keyframes;
};

struct SkeletalAnimation
{
	uint32_t	frameCount;
	float		duration;
	std::string name;
	std::vector<JointAnimation> jointAnimations;
};

class SkeletalHierarchy
{
public:
	std::vector<Joint> mJoints;

	SkeletalHierarchy();
	~SkeletalHierarchy();

	SkeletalHierarchy(const SkeletalHierarchy& other);
	SkeletalHierarchy(SkeletalHierarchy&& other);

	SkeletalHierarchy& operator=(const SkeletalHierarchy& other);
	SkeletalHierarchy& operator=(SkeletalHierarchy&& other);

	Joint* GetJointByName(const char* name);
	int GetJointIndexByName(const char* name);

	void UpdateAnimationPose();
	void CalculateSkinningMatrices(mat4f* skinningMatrices);
};


