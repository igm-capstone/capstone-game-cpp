#pragma once
#include <Vertex.h>
#include <Uniforms.h>


class SpriteManager
{
	Rig3D::Renderer*			mRenderer;
	Rig3D::IMesh*				mSpriteMesh;
	Rig3D::IShaderResource*		mSpritesShaderResource;

	int						mSheets = 0;
	int						mSprites = 0;
	const char*				mFilenames[MAX_SPRITESHEETS];
	CBuffer::SpriteSheet	mSpriteSheetData[MAX_SPRITESHEETS];
	GPU::Sprite				mSpriteInstanceData[MAX_SPRITES];

public:
	SpriteManager(): mRenderer(nullptr), mSpriteMesh(nullptr), mSpritesShaderResource(nullptr) {};
	~SpriteManager() {};

	void Initialize(Rig3D::IMesh* spriteMesh, Rig3D::IShaderResource* spriteShaderResource);

	void NewFrame() { mSprites = 0; }
	void LoadSpriteSheet(const char* filename, int spriteWidth, int spriteHeight, int slicesX, int slicesY);
	void DrawSprite(int sheetID, int spriteID, vec2f pos, vec2f size, vec2f linearFill = vec2f(1, 1), float radialFill = -1);
	void DrawSpriteAtPerc(int sheetID, int spriteID, vec2f screenPerc, vec2f size, vec2f linearFill = vec2f(1, 1), float radialFill = -1);

	const char** GetFilenames() { return mFilenames; };
	CBuffer::SpriteSheet* GetCBuffer() { return mSpriteSheetData; };
	GPU::Sprite* GetInstanceBuffer() { return mSpriteInstanceData; };
	int GetInstanceBufferCount() const { return mSprites; };
};