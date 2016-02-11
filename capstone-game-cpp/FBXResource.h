#pragma once
#include <fbxsdk.h>
#include <trace.h>

#define GET_FLOAT(f) static_cast<float>(f)

namespace
{
	FbxManager* gFbxMgr = nullptr;
}

template <class Vertex>
class FBXMeshResource
{
public:
	std::vector<Vertex>		mVertices;
	//std::vector<float[3]>	mPositions;
	//std::vector<float[3]>	mNormals;
	//std::vector<float[2]>	mUVs;
	std::vector<uint16_t>	mIndices;

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

		pImporter->Destroy();

		FbxNode* pRootNode = pScene->GetRootNode();

		if (pRootNode)
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
					int index;
					bool unmapped;

				//	TRACE_LOG("Polygon: " << pIndex);

					int directxindices[3] = { 2,1,0 };


					for (int vIndex = 0; vIndex < pSize; vIndex++)
					{
						index = pMesh->GetPolygonVertex(pIndex, directxindices[vIndex]);
						
						position = pMesh->GetControlPointAt(index);
						pMesh->GetPolygonVertexNormal(pIndex, index, normal);
						pMesh->GetPolygonVertexUV(pIndex, index, uvSetNameList.GetStringAt(0), uv, unmapped);

						Vertex vertex;
						vertex.Position = { GET_FLOAT(position.mData[0]), GET_FLOAT(position.mData[1]), GET_FLOAT(position.mData[2]) };
						vertex.Normal	= { GET_FLOAT(normal.mData[0]), GET_FLOAT(normal.mData[1]), GET_FLOAT(normal.mData[2]) };

						if (!unmapped)
						{
							vertex.UV = { GET_FLOAT(uv.mData[0]), GET_FLOAT(uv.mData[1]) };
						}

						mVertices.push_back(vertex);
						mIndices.push_back(indices++);

				//		TRACE_LOG("Position: " << index << " " << (float)position.mData[0] << " " << (float)position.mData[1] << " " << (float)position.mData[2]);
					}
				}
				//// Positions

				//for (int vIndex = 0; vIndex < pMesh->GetControlPointsCount(); vIndex++)
				//{
				//	FbxVector4 fbxVertex = pMesh->GetControlPointAt(j);
				//	mPositions.push_back({ fbxVertex.mData[0], fbxVertex.mData[1], fbxVertex.mData[2] });
				//}

				//// Normals
				//FbxGeometryElementNormal* pGeometryElementNormal = pMesh->GetElementNormal();
				//if (pGeometryElementNormal)
				//{
				//	if (pGeometryElementNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				//	{
				//		for (int vIndex = 0; vIndex < pMesh->GetControlPointsCount(); vIndex++)
				//		{
				//			int nIndex = 0;
				//			if (pGeometryElementNormal->GetReferenceMode() == FbxGeometryElement::eDirect)
				//			{
				//				nIndex = vIndex;
				//			}

				//			if (pGeometryElementNormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				//			{
				//				nIndex = pGeometryElementNormal->GetIndexArray().GetAt(vIndex);
				//			}

				//			FbxVector4 normal = pGeometryElementNormal->GetDirectArray().GetAt(nIndex);
				//			mNormals.push_back({ normal[0], normal[1], normal[2] });
				//		}
				//	}
				//	else if (pGeometryElementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				//	{
				//		int indexByPolygonVertex = 0;

				//		for (int pIndex = 0; pIndex < pMesh->GetPolygonCount(); pIndex++)
				//		{
				//			
				//			int pSize = pMesh->GetPolygonSize(pIndex);
				//			assert(pSize == 3);

				//			for (int vIndex = 0; vIndex < pSize; vIndex++)
				//			{
				//				int nIndex = 0;
				//				if (pGeometryElementNormal->GetReferenceMode() == FbxGeometryElement::eDirect)
				//				{
				//					nIndex = indexByPolygonVertex;
				//				}

				//				if (pGeometryElementNormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				//				{
				//					nIndex = pGeometryElementNormal->GetIndexArray().GetAt(indexByPolygonVertex);
				//				}

				//				FbxVector4 normal = pGeometryElementNormal->GetDirectArray().GetAt(nIndex);
				//				mNormals.push_back({ normal[0], normal[1], normal[2] });

				//				indexByPolygonVertex++;
				//			}
				//		}
				//	}
				//}

				//// UVs

				//for (int pIndex = 0; pIndex < pMesh->GetPolygonCount(); pIndex++)
				//{

				//	int pSize = pMesh->GetPolygonSize(pIndex);
				//	assert(pSize == 3);

				//	FbxStringList uvSetList;

				//	for (int vIndex = 0; vIndex < pSize; vIndex++)
				//	{
				//		pMesh->GetUVSetNames(uvSetList);

				//		pMesh->getuvs
				//	}
				//}

				//// Indices

				//int* pIndices = pMesh->GetPolygonVertices();
				//for (int j = 0; j < pMesh->GetPolygonVertexCount(); j++)
				//{
				//	mIndices.push_back(static_cast<uint16_t>(pIndices[j]));
				//}			
			}
		}

		return 1;
	}
};