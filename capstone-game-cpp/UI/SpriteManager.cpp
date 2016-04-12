#include <stdafx.h>
#include "SpriteManager.h"
#include <Rig3D/Graphics/Interface/IShaderResource.h>

void SpriteManager::Initialize(Rig3D::IMesh* spriteMesh, Rig3D::IShaderResource* spriteShaderResource)
{
	mRenderer = (Rig3D::Singleton<Rig3D::Engine>::SharedInstance()).GetRenderer();
	mSpritesShaderResource = spriteShaderResource;
	mSpriteMesh = spriteMesh;
}

void SpriteManager::LoadSpriteSheet(const char* filename, int spriteWidth, int spriteHeight, int slicesX, int slicesY)
{
	mFilenames[mSheets] = filename;

	mSpriteSheetData[mSheets].sheetWidth = spriteWidth*slicesX;
	mSpriteSheetData[mSheets].sheetHeight = spriteHeight*slicesY;
	mSpriteSheetData[mSheets].slicesX = slicesX;
	mSpriteSheetData[mSheets].slicesY = slicesY;
	
	mSheets++;
}

void SpriteManager::DrawSprite(int sheetID, int spriteID, vec2f pos, vec2f scale, vec2f linearFill, float radialFill)
{
	mSpriteInstanceData[mSprites].pointpos = pos;
	mSpriteInstanceData[mSprites].sheetID = sheetID;
	mSpriteInstanceData[mSprites].spriteID = spriteID;
	mSpriteInstanceData[mSprites].size = scale;
	mSpriteInstanceData[mSprites].linearFill = linearFill;
	mSpriteInstanceData[mSprites].radialFill = radialFill;
	
	mSprites++;
}

void SpriteManager::DrawSpriteAtPerc(int sheetID, int spriteID, vec2f screenPerc, vec2f scale, vec2f linearFill, float radialFill)
{
	auto pos = screenPerc;
	pos.x *= mRenderer->GetWindowWidth();
	pos.y *= mRenderer->GetWindowHeight();
	
	DrawSprite(sheetID, spriteID, pos, scale, linearFill, radialFill);
}