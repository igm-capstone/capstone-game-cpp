#pragma once
#include <Vertex.h>
#include <Uniforms.h>

enum SpriteSheetCode
{
	SPRITESHEET_BARS = 2,
	SPRITESHEET_GHOST_ICONS,
	SPRITESHEET_EXPLORER_ICONS,
	SPRITESHEET_PANELS,
	SPRITESHEET_CONTROL_ICONS
};


enum FontCode
{
	SPRITESHEET_FONT_NORMAL,
	SPRITESHEET_FONT_BOLD,
};

enum Alignment
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};


struct Glyph
{
	Glyph() {};
	/*Glyph(int id, int x, int y, int width, int height, float xoffset, float yoffset, float xadvance, int page, int chnl, int texWidth, int texHeight) :
		id(id), x(x), y(y), width(width), height(height), xoffset(xoffset), yoffset(yoffset), xadvance(xadvance)
	{
		/*sMin = static_cast<float>(x) / static_cast<float>(texWidth);
		tMin = static_cast<float>(y) / static_cast<float>(texHeight);

		sMax = static_cast<float>(x + width) / static_cast<float>(texWidth);
		tMax = static_cast<float>(y + height) / static_cast<float>(texHeight);*/
	//}

	int id;
	int x;
	int y;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;

	/*float sMin;
	float sMax;
	float tMin;
	float tMax;*/
};

typedef vec2f px;
typedef vec2f perc;
typedef vec2f ref;

class SpriteManager
{
	Rig3D::Renderer*			mRenderer;
	Rig3D::IMesh*				mSpriteMesh;
	Rig3D::IShaderResource*		mSpritesShaderResource;

	int						mSheets = 0;
	int						mFonts = 0;
	int						mSprites = 0;
	int						mGlyphs = 0;
	const char*				mFilenames[MAX_SPRITESHEETS];
	CBuffer::SpriteSheet	mSpriteSheetData[MAX_SPRITESHEETS];
	GPU::Sprite				mSpriteInstanceData[MAX_SPRITES];
	GPU::Glyph				mGlyphInstanceData[MAX_GLYPHS];
	Glyph					mGlyphsData[2][255];

	void DrawGlyphs(FontCode fontID, vec2f scale, char* phrase, vec2f pos, vec3f color, Alignment align);
	void DrawGlyph(FontCode fontID, vec2f scale, Glyph& glyph, vec2f pos, vec3f color);

public:
	SpriteManager(): mRenderer(nullptr), mSpriteMesh(nullptr), mSpritesShaderResource(nullptr) {};
	~SpriteManager() {};

	void Initialize(Rig3D::IMesh* spriteMesh, Rig3D::IShaderResource* spriteShaderResource);

	void NewFrame() { mSprites = 0; mGlyphs = 0; }
	void LoadSpriteSheet(const char* filename, int spriteWidth, int spriteHeight, int slicesX, int slicesY);
	void LoadFont(const char* filename);
	void DrawSprite(SpriteSheetCode sheetID, int spriteID, vec2f pos, vec2f size, vec2f linearFill = vec2f(1, 1), float radialFill = -1);
	void DrawTextSprite(FontCode fontID, float fontSize, vec2f pos, vec3f color, Alignment align, char* fmt, ...);
	
	vec2f perc2f(float x, float y) { return vec2f(x * mRenderer->GetWindowWidth(), y *mRenderer->GetWindowHeight()); };

	const char** GetTextureNames() { return mFilenames; };
	int GetTextureCount() { return mSheets; };
	CBuffer::SpriteSheet* GetCBuffer() { return mSpriteSheetData; };
	GPU::Sprite* GetSpriteInstanceBuffer() { return mSpriteInstanceData; };
	GPU::Glyph* GetGlyphInstanceBuffer() { return mGlyphInstanceData; };
	int GetSpriteInstanceBufferCount() const { return mSprites; };
	int GetGlyphInstanceBufferCount() const { return mGlyphs; };
};