#pragma once

void CreateWireFrameRasterizerState();
void CreateColliderMesh(void* allocator);
void RenderWallColliders(void* pShaderResource, void* pCameraManager, void* pModel);
void RenderGBuffer(void* pShaderResource);
void ReleaseGlobals();