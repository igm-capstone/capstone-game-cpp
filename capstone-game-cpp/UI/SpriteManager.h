#pragma once
#include <Vertex.h>
#include <Uniforms.h>

class SpriteManager
{
	class IMesh*				mSpriteMesh;
	class IShaderResource*		mSpritesShaderResource;

	int						mSheets = 0;
	int						mSprites = 0;
	const char*				mFilenames[MAX_SPRITESHEETS];
	CBuffer::SpriteSheet	mSpriteSheetData[MAX_SPRITESHEETS];
	GPU::Sprite				mSpriteInstanceData[MAX_SPRITES];


public:
	SpriteManager()	{};
	~SpriteManager()
	{
		
	};

	void Initialize(IMesh* spriteMesh, IShaderResource* spriteShaderResource);

	void NewFrame() { mSprites = 0; }
	void LoadSpriteSheet(const char* filename, int width, int height, int slicesX, int slicesY);
	void DrawSprite(int sheetID, int spriteID, vec2f pos, vec2f scale = vec2f(1,1), vec2f anchorScale = vec2f(1, 1));

	const char** GetFilenames() { return mFilenames; };
	CBuffer::SpriteSheet* GetCBuffer() { return mSpriteSheetData; };
	GPU::Sprite* GetInstanceBuffer() { return mSpriteInstanceData; };
	int GetInstanceBufferCount() { return mSprites; };

};
