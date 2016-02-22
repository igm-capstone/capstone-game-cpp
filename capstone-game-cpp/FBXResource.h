/* 
Resources:
http://docs.autodesk.com/FBX/2014/ENU/FBX-SDK-Documentation/index.html
http://www.walkerb.net/blog/dx-4/
http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/skinned-mesh-animation-using-matrices-r3577
http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/how-to-work-with-fbx-sdk-r3582
*/

#pragma once
#include <fbxsdk.h>
#include <SkeletalHierarchy.h>
#include "trace.h"

#define GET_FLOAT(f) static_cast<float>(f)

#define FBX_FPS FbxTime::eFrames30

namespace
{
	FbxManager* gFbxMgr = nullptr;
}

template <class Vertex>
class FBXMeshResource
{
public:
	std::unordered_map<int, std::vector<JointBlendWeight>>	mControlPointJointBlendMap;
	std::vector<Vertex>										mVertices;
	std::vector<uint16_t>									mIndices;

	SkeletalHierarchy										mSkeletalHierarchy;
	std::vector<SkeletalAnimation>							mSkeletalAnimations;

	const char*		mFilename;
	FbxImporter*	mImporter;

	FBXMeshResource(const char* filename) : mFilename(filename), mImporter(nullptr)
	{

	}

	FBXMeshResource() : FBXMeshResource(nullptr)
	{
			
	}

	~FBXMeshResource()
	{
		mControlPointJointBlendMap.clear();
		mVertices.clear();
		mIndices.clear();
		mSkeletalAnimations.clear();
	}

	void PrintNodesRecursively(FbxNode* node, int depth)
	{
		if (node->GetChildCount() == 0)
		{
			return;
		}

		for (int i = 0; i < depth; i++)
		{
			TRACE(" ");
		}

		TRACE(node->GetName());

		FbxNodeAttribute* pNodeAttribute = node->GetNodeAttribute();
		if (pNodeAttribute)
		{
			TRACE("\t" << pNodeAttribute->GetAttributeType() << Trace::endl);
		}
		else
		{
			TRACE(Trace::endl);
		}

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			PrintNodesRecursively(node->GetChild(i), depth + 1);
		}
	}

	int Load()
	{
		if (!gFbxMgr)
		{
			gFbxMgr = FbxManager::Create();

			FbxIOSettings* pIOSettings = FbxIOSettings::Create(gFbxMgr, IOSROOT);
			gFbxMgr->SetIOSettings(pIOSettings);
		}

		mImporter = FbxImporter::Create(gFbxMgr, "");
		bool success = mImporter->Initialize(mFilename, -1, gFbxMgr->GetIOSettings());
		if (!success)
		{
			return -1;
		}

		FbxScene* pScene = FbxScene::Create(gFbxMgr, "");
		success = mImporter->Import(pScene);
		if (!success)
		{
			return -1;
		}

		// Coordinate system conversion. This only affects the transform of the root node. 
		// To permeate this change throughout the mesh you need to incorporate the root transform.
		FbxAxisSystem axisSystem = pScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem dxAxisSystem(FbxAxisSystem::eDirectX);
		if (axisSystem != dxAxisSystem)
		{
			//dxAxisSystem.ConvertScene(pScene);
		}

		FbxNode* pRootNode = pScene->GetRootNode();

		PrintNodesRecursively(pRootNode, 0);


		if (pRootNode)
		{
			bool isRigged = LoadSkeletalHierarchy(pRootNode);
			if (isRigged)
			{
				LoadSkeletalAnimations(pRootNode);
			}

			LoadMesh(pRootNode);
		}

		mImporter->Destroy();

		return 1;
	}

	void LoadMesh(FbxNode* pRootNode)
	{
		int childNodeCount = pRootNode->GetChildCount();
		if (childNodeCount == 0)
		{
			return;
		}

		for (int i = 0; i < childNodeCount; i++)
		{
			LoadMeshRecusively(pRootNode->GetChild(i));
		}
	}

	void LoadMeshRecusively(FbxNode* pNode)
	{
		FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
		if (!pNodeAttribute)
		{
			return;
		}

		FbxNodeAttribute::EType AttributeType = pNode->GetNodeAttribute()->GetAttributeType();
		if (AttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = pNode->GetMesh();

			FbxVector4* pVertices = pMesh->GetControlPoints();

			FbxStringList uvSetNameList;
			pMesh->GetUVSetNames(uvSetNameList);

			uint16_t indices = 0;

			for (int pIndex = 0; pIndex < pMesh->GetPolygonCount(); pIndex++)
			{
				int pSize = pMesh->GetPolygonSize(pIndex);

				// We only draw triangles, feel me?
				assert(pSize == 3);

				FbxVector4 position, normal;
				FbxVector2 uv;
				int controlPointIndex;
				bool unmapped;

				int ccw[] = { 0, 1, 2 };

				for (int polygonVertexIndex = 0; polygonVertexIndex < pSize; polygonVertexIndex++)
				{
					controlPointIndex = pMesh->GetPolygonVertex(pIndex, ccw[polygonVertexIndex]);

					// Position
					position = pMesh->GetControlPointAt(controlPointIndex);

					// Normal
					pMesh->GetPolygonVertexNormal(pIndex, polygonVertexIndex, normal);

					// UV
					pMesh->GetPolygonVertexUV(pIndex, polygonVertexIndex, uvSetNameList.GetStringAt(0), uv, unmapped);

					Vertex vertex;
					vertex.SetPosition(vec3f(GET_FLOAT(position.mData[0]), GET_FLOAT(position.mData[1]), GET_FLOAT(position.mData[2])));
					vertex.SetNormal(vec3f(GET_FLOAT(normal.mData[0]), GET_FLOAT(normal.mData[1]), GET_FLOAT(normal.mData[2])));

					if (!unmapped)
					{
						vertex.SetUV(vec2f(GET_FLOAT(uv.mData[0]), GET_FLOAT(uv.mData[1])));
					}

					// Blend info

					std::vector<JointBlendWeight>& controlPointInfluenceJoints = mControlPointJointBlendMap[controlPointIndex];
					uint32_t controlPointInfluenceJointCount = min(controlPointInfluenceJoints.size(), 4);
					uint32_t diff = abs(static_cast<int>(4 - controlPointInfluenceJoints.size()));

					for (uint32_t i = 0; i < controlPointInfluenceJointCount; i++)
					{
						vertex.SetBlendIndices(i, controlPointInfluenceJoints[i].jointIndex);
						vertex.SetBlendWeights(i, controlPointInfluenceJoints[i].jointWeight);
					}

					// Set remaining indices to zero.
					for (uint32_t i = controlPointInfluenceJointCount; i < 4; i++)
					{
						vertex.SetBlendIndices(i, 0);
						vertex.SetBlendWeights(i, 0.0f);
					}

					mVertices.push_back(vertex);
					mIndices.push_back(indices++);
				}
			}
		}

		// Process children
		int childNodeCount = pNode->GetChildCount();
		for (int i = 0; i < childNodeCount; i++)
		{
			LoadMeshRecusively(pNode->GetChild(i));
		}
	}

	bool LoadSkeletalHierarchy(FbxNode* pRootNode)
	{
		int childNodeCount = pRootNode->GetChildCount();
		if (childNodeCount == 0)
		{
			return false;
		}

		for (int i = 0; i < childNodeCount; i++)
		{
			LoadSkeletalHierarchyRecursively(pRootNode->GetChild(i), 0, 0, -1);
		}

		return true;
	}

	void LoadSkeletalHierarchyRecursively(FbxNode* pNode, const int& depth, const int& index, const int& parentIndex)
	{
		FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
		if (!pNodeAttribute)
		{
			return;
		}

		FbxNodeAttribute::EType AttributeType = pNode->GetNodeAttribute()->GetAttributeType();
		if (AttributeType == FbxNodeAttribute::eSkeleton)
		{
			// Add joint to skeleton
			mSkeletalHierarchy.mJoints.push_back(Joint());

			// Set Joint name and parent index
			Joint* pJoint = &mSkeletalHierarchy.mJoints.back();
			pJoint->name = pNode->GetName();
			pJoint->parentIndex = parentIndex;
		}

		// Process children
		int childNodeCount = pNode->GetChildCount();
		for (int i = 0; i < childNodeCount; i++)
		{
			LoadSkeletalHierarchyRecursively(pNode->GetChild(i), depth + 1, mSkeletalHierarchy.mJoints.size(), index);
		}
	}

	void LoadSkeletalAnimations(FbxNode* pRootNode)
	{
		int childNodeCount = pRootNode->GetChildCount();
		if (childNodeCount == 0)
		{
			return;
		}

		for (int i = 0; i < childNodeCount; i++)
		{
			LoadSkeletalAnimationsRecursively(pRootNode->GetChild(i));
		}
	}

	void LoadSkeletalAnimationsRecursively(FbxNode* pNode)
	{
		if (!pNode->GetNodeAttribute())
		{
			return;
		}

		FbxNodeAttribute::EType AttributeType = pNode->GetNodeAttribute()->GetAttributeType();
		if (AttributeType == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = pNode->GetMesh();

			// Geometry matrix: Most likely identity
			FbxVector4 scale = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
			FbxVector4 rotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
			FbxVector4 translation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			FbxAMatrix geometryMatrix = FbxAMatrix(translation, rotation, scale);

			// Get Skin Info
			int deformerCount = pMesh->GetDeformerCount();

			for (int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++)
			{
				FbxSkin* pSkin = reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
				if (!pSkin)
				{
					continue;
				}

				// Get Joint Animation Info (Keyframes per animation per joint)
				// Each Skeletal Animation is composed of a set of keyframe per joint (Joint Animation)
				int animStackCount = mImporter->GetAnimStackCount();

				for (int animStackIndex = 0; animStackIndex < animStackCount; animStackIndex++)
				{
					FbxTakeInfo* pTakeInfo = mImporter->GetTakeInfo(animStackIndex);
					FbxTime startTime = pTakeInfo->mLocalTimeSpan.GetStart();
					FbxTime endTime = pTakeInfo->mLocalTimeSpan.GetStop();
					FbxLongLong frameCount = endTime.GetFrameCount(FBX_FPS) - startTime.GetFrameCount(FBX_FPS);
					FbxLongLong duration = endTime.GetMilliSeconds() - startTime.GetMilliSeconds();

					// Add a skeletal animation (full character animation)
					mSkeletalAnimations.push_back(SkeletalAnimation());

					// Get pointer to current skeletal animation and set duration
					SkeletalAnimation* pSkeletalAnimation = &mSkeletalAnimations.back();
					pSkeletalAnimation->frameCount = static_cast<uint32_t>(frameCount);
					pSkeletalAnimation->duration = static_cast<float>(duration);
					pSkeletalAnimation->name = pTakeInfo->mName.Buffer();

					// Get Joint Info
					int clusterCount = pSkin->GetClusterCount();

					for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
					{
						FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);

						FbxAMatrix transformMatrix, transformLinkMatrix;
						pCluster->GetTransformMatrix(transformMatrix);				// Model Space Transform (Mesh Transform) This is the cluster transform w/r/t the Model. I believe we need this if we convert the scene coordinate system.
						pCluster->GetTransformLinkMatrix(transformLinkMatrix);		// Global Space Transform. This is the cluster transform w/r/t the Scene. This traverses each parent.

																					// Transposing for graphics math
						FbxAMatrix inverseBindPoseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryMatrix; // Joint Space 

						int jointIndex = mSkeletalHierarchy.GetJointIndexByName(pCluster->GetLink()->GetName());
						assert(jointIndex > -1);

						Joint* pJoint = &mSkeletalHierarchy.mJoints[jointIndex];
						pJoint->inverseBindPoseMatrix.u = { GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[3]) };
						pJoint->inverseBindPoseMatrix.v = { GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[3]) };
						pJoint->inverseBindPoseMatrix.w = { GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[3]) };
						pJoint->inverseBindPoseMatrix.t = { GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[3]) };

						// Get control points influenced by this joint
						int controlPointIndexCount = pCluster->GetControlPointIndicesCount();
						int* controlPointIndices = pCluster->GetControlPointIndices();
						double* controlPointWeights = pCluster->GetControlPointWeights();

						for (int controlPointIndex = 0; controlPointIndex < controlPointIndexCount; controlPointIndex++)
						{
							mControlPointJointBlendMap[controlPointIndices[controlPointIndex]].push_back({ jointIndex, GET_FLOAT(controlPointWeights[controlPointIndex]) });
						}

						// Add a joint animation to the current skeletal animation
						pSkeletalAnimation->jointAnimations.push_back(JointAnimation());

						// Get pointer to current joint animation and set index
						JointAnimation* pJointAnimation = &pSkeletalAnimation->jointAnimations.back();
						pJointAnimation->jointIndex = jointIndex;

						// Get each frame for the joint animation
						for (FbxLongLong frameIndex = startTime.GetFrameCount(FBX_FPS); frameIndex <= endTime.GetFrameCount(FBX_FPS); frameIndex++)
						{
							FbxTime currentTime;
							currentTime.SetFrame(frameIndex, FBX_FPS);

							FbxAMatrix animModelMatrix = pNode->EvaluateGlobalTransform(currentTime) * geometryMatrix;							// Model space pose matrix. This incorporates the transform of coordinate system if we converted the scene earlier.
							FbxAMatrix animLinkMatrix = animModelMatrix.Inverse() * pCluster->GetLink()->EvaluateGlobalTransform(currentTime);	// Global Animated Space. This is the animated joint position w/r/t the scene.

							FbxQuaternion clusterRotation = animLinkMatrix.GetQ();
							FbxVector4 clusterScale = animLinkMatrix.GetS();
							FbxVector4 clusterTranslation = animLinkMatrix.GetT();

							// Add a key frame to the current joint animation
							pJointAnimation->keyframes.push_back(Keyframe());

							// Get pointer to current key frame and set properties
							Keyframe* keyframe = &pJointAnimation->keyframes.back();
							keyframe->rotation = { GET_FLOAT(clusterRotation.mData[3]), GET_FLOAT(clusterRotation.mData[0]), GET_FLOAT(clusterRotation.mData[1]), GET_FLOAT(clusterRotation.mData[2]) };
							keyframe->scale = { GET_FLOAT(clusterScale.mData[0]), GET_FLOAT(clusterScale.mData[1]), GET_FLOAT(clusterScale.mData[2]) };
							keyframe->translation = { GET_FLOAT(clusterTranslation.mData[0]), GET_FLOAT(clusterTranslation.mData[1]), GET_FLOAT(clusterTranslation.mData[2]) };
							keyframe->time = GET_FLOAT(currentTime.GetMilliSeconds());
						}
					}
				}
			}
		}

		// Process children
		int childNodeCount = pNode->GetChildCount();
		for (int i = 0; i < childNodeCount; i++)
		{
			LoadSkeletalAnimationsRecursively(pNode->GetChild(i));
		}
	}
};