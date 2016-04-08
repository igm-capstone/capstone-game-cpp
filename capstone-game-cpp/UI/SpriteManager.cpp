#include <stdafx.h>
#include "SpriteManager.h"
#include <Rig3D/Graphics/Interface/IShaderResource.h>

void SpriteManager::Initialize(IMesh* spriteMesh, IShaderResource* spriteShaderResource)
{
	mSpritesShaderResource = spriteShaderResource;
	mSpriteMesh = spriteMesh;
}

void SpriteManager::LoadSpriteSheet(const char* filename, int width, int height, int slicesX, int slicesY)
{
	mFilenames[mSheets] = filename;

	mSpriteSheetData[mSheets].sheetWidth = width;
	mSpriteSheetData[mSheets].sheetHeight = height;
	mSpriteSheetData[mSheets].slicesX = slicesX;
	mSpriteSheetData[mSheets].slicesY = slicesY;
	
	mSheets++;
}

void SpriteManager::DrawSprite(int sheetID, int spriteID, vec2f pos, vec2f scale, vec2f anchorScale)
{
	mSpriteInstanceData[mSprites].pointpos = pos;
	mSpriteInstanceData[mSprites].sheetID = sheetID;
	mSpriteInstanceData[mSprites].spriteID = spriteID;
	mSpriteInstanceData[mSprites].scale = scale;
	mSpriteInstanceData[mSprites].anchorScale = anchorScale;
	
	mSprites++;
}
