#pragma once
#include <fbxsdk.h>

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
					int controlPointIndex;
					bool unmapped;

					// Remapping for winding order to clockwise
					int cw[3]	= { 0, 2, 1 };

					for (int polygonVertexIndex = 0; polygonVertexIndex < pSize; polygonVertexIndex++)
					{
						controlPointIndex = pMesh->GetPolygonVertex(pIndex, cw[polygonVertexIndex]);
						
						// Position
						position = pMesh->GetControlPointAt(controlPointIndex);

						// Normal
						pMesh->GetPolygonVertexNormal(pIndex, cw[polygonVertexIndex], normal);
						
						// UV
						pMesh->GetPolygonVertexUV(pIndex, cw[polygonVertexIndex], uvSetNameList.GetStringAt(0), uv, unmapped);

						Vertex vertex;
						vertex.Position = { GET_FLOAT(position.mData[0]), GET_FLOAT(position.mData[1]), -GET_FLOAT(position.mData[2]) };
						vertex.Normal	= { GET_FLOAT(normal.mData[0]), GET_FLOAT(normal.mData[1]), -GET_FLOAT(normal.mData[2]) };

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

		return 1;
	}
};