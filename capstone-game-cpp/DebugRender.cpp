#include <stdafx.h>
#include <DebugRender.h>
#include <Rig3D/Rig3D/Engine.h>
#include "Vertex.h"
#include <Rig3D/Geometry.h>
#include <Rig3D/Graphics/MeshLibrary.h>
#include <Rig3D/Memory/Memory/Memory.h>
#include "SceneObjects/StaticCollider.h"
#include  <ScareTacticsApplication.h>
#include "SceneObjects/Region.h"
#include "Components/Skill.h"
#include "SceneObjects/Explorer.h"

namespace
{
	Rig3D::IMesh* gColliderMesh = nullptr;
	Rig3D::IMesh* gSphereMesh = nullptr;
	ID3D11RasterizerState* gWireframeRS = nullptr;
	Rig3D::Renderer* gRenderer = nullptr;
	Application* gApplication = nullptr;
}

void CreateWireFrameRasterizerState()
{
	gRenderer = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetRenderer();

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthClipEnable = true;

	gRenderer->GetDevice()->CreateRasterizerState(&rasterizerDesc, &gWireframeRS);
}

void CreateColliderMesh(void* allocator)
{
	Rig3D::MeshLibrary<LinearAllocator> meshLibrary(reinterpret_cast<LinearAllocator*>(allocator));
	std::vector<GPU::Vertex3> vertices;
	std::vector<uint16_t> indices;

	Rig3D::Geometry::Cube(vertices, indices, 2);

	meshLibrary.NewMesh(&gColliderMesh, gRenderer);
	gRenderer->VSetMeshVertexBuffer(gColliderMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	gRenderer->VSetMeshIndexBuffer(gColliderMesh, &indices[0], indices.size());

	vertices.clear();
	indices.clear();

	Rig3D::Geometry::Sphere(vertices, indices, 6, 6, 0.5f);

	meshLibrary.NewMesh(&gSphereMesh, gRenderer);
	gRenderer->VSetMeshVertexBuffer(gSphereMesh, &vertices[0], sizeof(GPU::Vertex3) * vertices.size(), sizeof(GPU::Vertex3));
	gRenderer->VSetMeshIndexBuffer(gSphereMesh, &indices[0], indices.size());
}

void RenderWallColliders(void* pShaderResource, void* pCameraManager, void* pModel)
{
	if (!gApplication)
	{
		gApplication = &Application::SharedInstance();
	}

	gRenderer->GetDeviceContext()->RSSetState(gWireframeRS);

	gRenderer->VSetInputLayout(gApplication->mExplorerVertexShader);
	gRenderer->VSetVertexShader(gApplication->mExplorerVertexShader);
	gRenderer->VSetPixelShader(gApplication->mExplorerPixelShader);

	IShaderResource* iShaderResource = reinterpret_cast<IShaderResource*>(pShaderResource);
	CameraManager* cameraManager = reinterpret_cast<CameraManager*>(pCameraManager);
	CBuffer::Model* model = reinterpret_cast<CBuffer::Model*>(pModel);

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, cameraManager->GetCBufferPersp(), 0);
	gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 0, 0);

	gRenderer->VBindMesh(gColliderMesh);

	for (Region& e : Factory<Region>())
	{
	//	if (e.mBoxCollider->mLayer != COLLISION_LAYER_WALL) continue;
		model->world = e.mTransform->GetWorldMatrix().transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VBindMesh(gSphereMesh);

	for (Explorer& s : Factory<Explorer>())
	{
		if (s.mMeleeColliderComponent.asSphereColliderComponent)
		{
			SphereColliderComponent* c = s.mMeleeColliderComponent.asSphereColliderComponent;
			model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
			gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
			gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

			gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());
		}
	}

	gRenderer->GetDeviceContext()->RSSetState(nullptr);
}

void ReleaseGlobals()
{
	gColliderMesh->~IMesh();
	gSphereMesh->~IMesh();
	ReleaseMacro(gWireframeRS);
}