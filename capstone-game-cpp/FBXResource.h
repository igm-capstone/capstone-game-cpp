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

#define GET_FLOAT(f) static_cast<float>(f)

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

	const char* mFilename;

	FBXMeshResource(const char* filename) : mFilename(filename)
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

		FbxImporter* pImporter = FbxImporter::Create(gFbxMgr, "");
		bool success = pImporter->Initialize(mFilename, -1, gFbxMgr->GetIOSettings());
		if (!success)
		{
			return -1;
		}

		FbxScene* pScene = FbxScene::Create(gFbxMgr, "");
		success = pImporter->Import(pScene);
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

		pImporter->Destroy();

		FbxNode* pRootNode = pScene->GetRootNode();

		if (pRootNode)
		{
			LoadMesh(pRootNode);

			bool isRigged = LoadSkeletalHierarchy(pRootNode);
			if (isRigged)
			{
				
			}


		}

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

		// Set Joint name and parent index
		Joint joint;
		joint.mName = pNode->GetName();
		joint.mParentIndex = parentIndex;

		// Add joint to skeleton
		mSkeleton.mJoints.push_back(joint);
		
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
		
	}

	void LoadSkeletalHierarchyRecursively(FbxNode* pNode)
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
		FbxVector4 rotation		= pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxVector4 translation	= pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix modelMatrix	= FbxAMatrix(translation, rotation, scale);
		
		int deformerCount = pMesh->GetDeformerCount();

		for (int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++)
		{
			FbxSkin* pSkin = reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!pSkin)
			{
				continue;
			}

			int clusterCount = pSkin->GetClusterCount();

			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
			{
				FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);

				FbxAMatrix transformMatrix, transformLinkMatrix;
				pCluster->GetTransformMatrix(transformMatrix);
				pCluster->GetTransformLinkMatrix(transformLinkMatrix);

				FbxAMatrix inverseBindPoseMatrix = transformLinkMatrix.Inverse() * transformMatrix * modelMatrix;

				int jointIndex = mSkeleton.GetJointIndexByName(pCluster->GetName());
				assert(jointIndex > -1);

				Joint* pJoint = &mSkeleton.mJoints[jointIndex];

				// May have to transpose this matrix...

				pJoint->inverseBindPoseMatrix.u = { GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(0).mData[3]) };
				pJoint->inverseBindPoseMatrix.v = { GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(1).mData[3]) };
				pJoint->inverseBindPoseMatrix.w = { GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(2).mData[3]) };
				pJoint->inverseBindPoseMatrix.t = { GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[0]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[1]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[2]), GET_FLOAT(inverseBindPoseMatrix.GetRow(3).mData[3]) };

				int controlPointIndexCount = pCluster->GetControlPointIndicesCount();

				for (int controlPointIndex = 0; controlPointIndex < controlPointIndexCount; controlPointIndex++)
				{
					mControlPointJointBlendMap[controlPointIndex].push_back({ jointIndex, GET_FLOAT(pCluster->GetControlPointWeights()[controlPointIndex]) });
				}

				FbxAnimStack* pAnimStack = pNode->GetScene()->GetCurrentAnimationStack();
				
				const char* animStackName = pAnimStack->GetName();

				FbxTakeInfo* pTakeInfo	= pNode->GetScene()->GetTakeInfo(animStackName);
				FbxTime startTime		= pTakeInfo->mLocalTimeSpan.GetStart();
				FbxTime endTime			= pTakeInfo->mLocalTimeSpan.GetStop();
				FbxLongLong duration	= endTime.GetFrameCount(FbxTime::eFrames24) - startTime.GetFrameCount(FbxTime::eFrames24) + 1;

			}
		}

	}

};