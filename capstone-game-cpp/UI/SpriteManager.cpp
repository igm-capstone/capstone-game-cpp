#include <stdafx.h>
#include "SpriteManager.h"
#include <Rig3D/Graphics/Interface/IShaderResource.h>

void SpriteManager::Initialize(Rig3D::IMesh* spriteMesh, Rig3D::IShaderResource* spriteShaderResource)
{
	mRenderer = (Rig3D::Singleton<Rig3D::Engine>::SharedInstance()).GetRenderer();
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

void SpriteManager::DrawSpriteAtPerc(int sheetID, int spriteID, vec2f screenPerc, vec2f scale, vec2f anchorScale)
{
	auto pos = screenPerc;
	pos.x *= mRenderer->GetWindowWidth();
	pos.y *= mRenderer->GetWindowHeight();
	
	DrawSprite(sheetID, spriteID, pos, scale, anchorScale);
}