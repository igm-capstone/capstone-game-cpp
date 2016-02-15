/* 
Resources:
http://docs.autodesk.com/FBX/2014/ENU/FBX-SDK-Documentation/index.html
http://www.walkerb.net/blog/dx-4/
http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/skinned-mesh-animation-using-matrices-r3577
http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/how-to-work-with-fbx-sdk-r3582
*/

#pragma once
#include <fbxsdk.h>
#include <Skeleton.h>
#include <Components/AnimationController.h>

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

	Skeleton												mSkeleton;
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

		// Coordinate system conversion
		FbxAxisSystem axisSystem = pScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem dxAxisSystem(FbxAxisSystem::eDirectX);
		if (axisSystem != dxAxisSystem)
		{
			dxAxisSystem.ConvertScene(pScene);
		}

		FbxNode* pRootNode = pScene->GetRootNode();

		if (pRootNode)
		{
			LoadMesh(pRootNode);

			bool isRigged = LoadSkeletalHierarchy(pRootNode);
			if (isRigged)
			{
				LoadSkeletalAnimations(pRootNode);
			}


		}

		mImporter->Destroy();

		return 1;
	}

	void LoadMesh(FbxNode* pRootNode)
	{
		for (int cIndex = 0; cIndex < pRootNode->GetChildCount(); cIndex++)
		{
			FbxNode* pChildNode = pRootNode->GetChild(cIndex);
			if (!pChildNode->GetNodeAttribute())
			{
				continue;
			}

			FbxNodeAttribute::EType AttributeType = pChildNode->GetNodeAttribute()->GetAttributeType();
			if (AttributeType != FbxNodeAttribute::eMesh)
			{
				continue;
			}

			FbxMesh* pMesh = reinterpret_cast<FbxMesh*>(pChildNode->GetNodeAttribute());

			FbxVector4* pVertices = pMesh->GetControlPoints();

			FbxStringList uvSetNameList;
			pMesh->GetUVSetNames(uvSetNameList);

			uint16_t indices = 0;

			for (int pIndex = 0; pIndex < pMesh->GetPolygonCount(); pIndex++)
			{
				int pSize = pMesh->GetPolygonSize(pIndex);
				assert(pSize == 3);

				FbxVector4 position, normal;
				FbxVector2 uv;
				int controlPointIndex;
				bool unmapped;

				for (int polygonVertexIndex = 0; polygonVertexIndex < pSize; polygonVertexIndex++)
				{
					controlPointIndex = pMesh->GetPolygonVertex(pIndex, polygonVertexIndex);

					// Position
					position = pMesh->GetControlPointAt(controlPointIndex);

					// Normal
					pMesh->GetPolygonVertexNormal(pIndex, polygonVertexIndex, normal);

					// UV
					pMesh->GetPolygonVertexUV(pIndex, polygonVertexIndex, uvSetNameList.GetStringAt(0), uv, unmapped);

					Vertex vertex;
					vertex.Position = { GET_FLOAT(position.mData[0]), GET_FLOAT(position.mData[1]), GET_FLOAT(position.mData[2]) };
					vertex.Normal = { GET_FLOAT(normal.mData[0]), GET_FLOAT(normal.mData[1]), GET_FLOAT(normal.mData[2]) };

					if (!unmapped)
					{
						vertex.UV = { GET_FLOAT(uv.mData[0]), GET_FLOAT(uv.mData[1]) };
					}

					mVertices.push_back(vertex);
					mIndices.push_back(indices++);
				}
			}
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
		if (AttributeType != FbxNodeAttribute::eSkeleton)
		{
			return;
		}

		// Add joint to skeleton
		mSkeleton.mJoints.push_back(Joint());

		// Set Joint name and parent index
		Joint* pJoint = &mSkeleton.mJoints.back();
		pJoint->name = pNode->GetName();
		pJoint->parentIndex = parentIndex;
		
		//for (int i = 0; i < depth; ++i)
		//{
		//	TRACE(" ");
		//}

		//TRACE(joint.mName << " " << joint.mParentIndex);
		//TRACE(Trace::endl);

		// Process children
		int childNodeCount = pNode->GetChildCount();
		for (int i = 0; i < childNodeCount; i++)
		{
			LoadSkeletalHierarchyRecursively(pNode->GetChild(i), depth + 1, mSkeleton.mJoints.size(), index);
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
		if (AttributeType != FbxNodeAttribute::eMesh)
		{
			return;
		}

		FbxMesh* pMesh = pNode->GetMesh();

		// Model matrix: Most likely identity
		FbxVector4 scale		= pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxVector4 rotation		= pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 translation	= pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxAMatrix modelMatrix	= FbxAMatrix(translation, rotation, scale);

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
				FbxTakeInfo* pTakeInfo	= mImporter->GetTakeInfo(animStackIndex);
				FbxTime startTime		= pTakeInfo->mLocalTimeSpan.GetStart();
				FbxTime endTime			= pTakeInfo->mLocalTimeSpan.GetStop();
				FbxLongLong duration	= endTime.GetFrameCount(FBX_FPS) - startTime.GetFrameCount(FBX_FPS) + 1;

				// Add a skeletal animation (full character animation)
				mSkeletalAnimations.push_back(SkeletalAnimation());

				// Get pointer to current skeletal animation and set duration
				SkeletalAnimation* pSkeletalAnimation	= &mSkeletalAnimations.back();
				pSkeletalAnimation->duration			= static_cast<float>(duration);
				pSkeletalAnimation->name				= pTakeInfo->mName.Buffer();

				// Get Joint Info
				int clusterCount = pSkin->GetClusterCount();

				for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
				{
					FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);

					FbxAMatrix transformMatrix, transformLinkMatrix;
					pCluster->GetTransformMatrix(transformMatrix);				// Mesh transform at bind time... hopefully identity
					pCluster->GetTransformLinkMatrix(transformLinkMatrix);		// Transforms from joint to model space

					// Transposing for graphics math
					FbxAMatrix inverseBindPoseMatrix = (transformLinkMatrix.Inverse() * transformMatrix * modelMatrix).Transpose();

					int jointIndex = mSkeleton.GetJointIndexByName(pCluster->GetLink()->GetName());
					assert(jointIndex > -1);

					Joint* pJoint = &mSkeleton.mJoints[jointIndex];

					pJoint->inverseBindPoseMatrix.u = { GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[3]) };
					pJoint->inverseBindPoseMatrix.v = { GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[3]) };
					pJoint->inverseBindPoseMatrix.w = { GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[3]) };
					pJoint->inverseBindPoseMatrix.t = { GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[3]) };

					// Get control points influenced by this joint
					int controlPointIndexCount = pCluster->GetControlPointIndicesCount();

					for (int controlPointIndex = 0; controlPointIndex < controlPointIndexCount; controlPointIndex++)
					{
						mControlPointJointBlendMap[controlPointIndex].push_back({ jointIndex, GET_FLOAT(pCluster->GetControlPointWeights()[controlPointIndex]) });
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

						FbxAMatrix animModelMatrix	= pNode->EvaluateGlobalTransform(currentTime) * modelMatrix;								// Transforms to world space
						FbxAMatrix animLinkMatrix	= animModelMatrix.Inverse() * pCluster->GetLink()->EvaluateGlobalTransform(currentTime);	// Transforms cluster from world space to model space

						FbxVector4 clusterScale			= animLinkMatrix.GetS();
						FbxQuaternion clusterRotation	= animLinkMatrix.GetQ();
						FbxVector4 clusterTranslation	= animLinkMatrix.GetT();

						// Add a key frame to the current joint animation
						pJointAnimation->keyframes.push_back(Keyframe());

						// Get pointer to current key frame and set properties
						Keyframe* keyframe		= &pJointAnimation->keyframes.back();
						keyframe->rotation		= { GET_FLOAT(clusterRotation.mData[3]), GET_FLOAT(clusterRotation.mData[0]), GET_FLOAT(clusterRotation.mData[1]), GET_FLOAT(clusterRotation.mData[2]) };
						keyframe->scale			= { GET_FLOAT(clusterScale.mData[0]), GET_FLOAT(clusterScale.mData[1]), GET_FLOAT(clusterScale.mData[2]) };
						keyframe->translation	= { GET_FLOAT(clusterTranslation.mData[0]), GET_FLOAT(clusterTranslation.mData[1]), GET_FLOAT(clusterTranslation.mData[2]) };
						keyframe->time			= GET_FLOAT(currentTime.GetMilliSeconds());
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