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
#include "SceneObjects/Explorer.h"
#include "SceneObjects/Door.h"
#include "SceneObjects/Lamp.h"
#include "SceneObjects/StaticMesh.h"
#include "SceneObjects/Minion.h"

namespace
{
	Rig3D::IMesh* gColliderMesh = nullptr;
	Rig3D::IMesh* gSphereMesh = nullptr;
	ID3D11RasterizerState* gWireframeRS = nullptr;
	Rig3D::Renderer* gRenderer = nullptr;
	Rig3D::Input*	gInput = nullptr;
	Application* gApplication = nullptr;
}

void CreateWireFrameRasterizerState()
{
	if (!gRenderer)
	{
		gRenderer = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetRenderer();
	}

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

	Rig3D::Geometry::Sphere(vertices, indices, 6, 6, 1.0f);

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

	gRenderer->VSetInputLayout(gApplication->mVSDefSingleColor);
	gRenderer->VSetVertexShader(gApplication->mVSDefSingleColor);
	gRenderer->VSetPixelShader(gApplication->mPSDefColor);

	IShaderResource* iShaderResource = reinterpret_cast<IShaderResource*>(pShaderResource);
	CameraManager* cameraManager = reinterpret_cast<CameraManager*>(pCameraManager);
	CBuffer::Model* model = reinterpret_cast<CBuffer::Model*>(pModel);

	vec4f color[6] = { Colors::green, Colors::cyan, Colors::blue, Colors::white, Colors::black, Colors::yellow };

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, cameraManager->GetCBufferPersp(), 0);
	gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 0, 0);

	gRenderer->VBindMesh(gColliderMesh);

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[0], 3);
	for (Region& e : Factory<Region>())
	{
		model->world = e.mTransform->GetWorldMatrix().transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[1], 3);
	for (StaticCollider& e : Factory<StaticCollider>())
	{
		model->world = e.mTransform->GetWorldMatrix().transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[2], 3);
	for (Door& e : Factory<Door>())
	{
		if (!e.mColliderComponent->mIsActive) continue;
		model->world = (mat4f::scale(e.mColliderComponent->mCollider.halfSize * 2.0f) * mat4f::translate(e.mColliderComponent->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[3], 3);
	for (Door& e : Factory<Door>())
	{
		if (!e.mTrigger->mIsActive) continue;
		model->world = (mat4f::scale(e.mTrigger->mCollider.halfSize * 2.0f) * mat4f::translate(e.mTrigger->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}
	for (Lamp& e : Factory<Lamp>())
	{
		if (!e.mTrigger->mIsActive) continue;
		model->world = (mat4f::scale(e.mTrigger->mCollider.halfSize * 2.0f) * mat4f::translate(e.mTrigger->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[5], 3);
	for (StaticMesh& e : Factory<StaticMesh>())
	{
		if (!e.mColliderComponent->mIsActive) continue;
		model->world = (mat4f::scale(e.mColliderComponent->mCollider.halfSize * 2.0f) * mat4f::translate(e.mColliderComponent->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 3, 2);

		gRenderer->VDrawIndexed(0, gColliderMesh->GetIndexCount());
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[3], 3);

	gRenderer->VBindMesh(gSphereMesh);

	for (Explorer& s : Factory<Explorer>())
	{
		SphereColliderComponent* c = s.mCollider;
		model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

		gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());

		c = s.mInteractionCollider;
		model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

		gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());

		if (s.mMeleeColliderComponent.asSphereColliderComponent)
		{
			c = s.mMeleeColliderComponent.asSphereColliderComponent;
			model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
			gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
			gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

			gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());
		}
	}

	for (Minion& m : Factory<Minion>())
	{
		SphereColliderComponent* c = m.mCollider;
		model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

		gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());

		if (m.mMeleeColliderComponent)
		{
			c = m.mMeleeColliderComponent;
			model->world = (mat4f::scale(c->mCollider.radius) * mat4f::translate(c->mCollider.origin)).transpose();
			gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
			gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

			gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());
		}
	}

	gRenderer->VUpdateShaderConstantBuffer(iShaderResource, &color[4], 3);
	
	mat4f defaultRotation = mat4f::rotateZ(PI * 0.5f);

	gRenderer->VBindMesh(gSphereMesh);
	for (Lamp& l : Factory<Lamp>())
	{
		
		model->world = (mat4f::scale(l.mLightRadius) * l.mTransform->GetWorldMatrix()).transpose();
		gRenderer->VUpdateShaderConstantBuffer(iShaderResource, model, 1);
		gRenderer->VSetVertexShaderConstantBuffer(iShaderResource, 1, 1);

		gRenderer->VDrawIndexed(0, gSphereMesh->GetIndexCount());
	}

	gRenderer->GetDeviceContext()->RSSetState(nullptr);
}

void RenderGBuffer(void* pRenderContext)
{
	static int gBufferIndex = 0;

	if (!gApplication)
	{
		gApplication = &Application::SharedInstance();
	}

	if (!gRenderer)
	{
		gRenderer = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetRenderer();
	}

	if (!gInput)
	{
		gInput = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetInput();
	}

	if (gInput->GetKeyDown(KEYCODE_RIGHT))
	{
		gBufferIndex = min(gBufferIndex + 1, 4);
	}
	else if (gInput->GetKeyDown(KEYCODE_LEFT))
	{
		gBufferIndex = max(0, gBufferIndex - 1);
	}

	gRenderer->VSetContextTarget();
	gRenderer->VClearContextTarget(Colors::magenta.pCols);

	gRenderer->VSetInputLayout(gApplication->mVSFwdFullScreenQuad);
	gRenderer->VSetVertexShader(gApplication->mVSFwdFullScreenQuad);
	gRenderer->VSetPixelShader(gApplication->mPSDef2DTexture);

	IRenderContext* pCtx = reinterpret_cast<IRenderContext*>(pRenderContext);

	if (gBufferIndex < 4)
	{
		gRenderer->VSetPixelShaderResourceView(pCtx, gBufferIndex, 0);	
	}
	else
	{
		gRenderer->VSetPixelShaderDepthResourceView(pCtx, 0, 0);
	}

	gRenderer->GetDeviceContext()->Draw(3, 0);
}

void ReleaseGlobals()
{
	gColliderMesh->~IMesh();
	gSphereMesh->~IMesh();
	ReleaseMacro(gWireframeRS);
}