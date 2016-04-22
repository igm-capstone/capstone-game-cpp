#include <stdafx.h>
#include "SpriteManager.h"
#include <Rig3D/Graphics/Interface/IShaderResource.h>
#include <trace.h>
#include <Colors.h>

void SpriteManager::Initialize(Rig3D::IMesh* spriteMesh, Rig3D::IShaderResource* spriteShaderResource)
{
	mRenderer = (Rig3D::Singleton<Rig3D::Engine>::SharedInstance()).GetRenderer();
	mSpritesShaderResource = spriteShaderResource;
	mSpriteMesh = spriteMesh;

	LoadFont("Assets/UI/Fonts/ashcanbb_reg.ttf_sdf.png");
	LoadFont("Assets/UI/Fonts/ashcanbb_bold.ttf_sdf.png");
	LoadSpriteSheet("Assets/UI/Health.png", 900, 112, 1, 4);
	LoadSpriteSheet("Assets/UI/UI_ghostIcons.png", 256, 256, 4, 4);
	LoadSpriteSheet("Assets/UI/UI_playerIcons1024.png", 256, 256, 4, 4);
	LoadSpriteSheet("Assets/UI/Panels.png", 1024, 256, 1, 4);
	LoadSpriteSheet("Assets/UI/Keys.png", 96, 96, 8, 2);
	LoadSpriteSheet("Assets/UI/Icons.png", 256, 256, 4, 4);
	LoadSpriteSheet("Assets/UI/Start0.png", 1024, 1024, 1, 1);
	LoadSpriteSheet("Assets/UI/Start1.png", 1024, 1024, 1, 1);
	LoadSpriteSheet("Assets/UI/Start2.png", 1024, 1024, 1, 1);
	LoadSpriteSheet("Assets/UI/Start3.png", 1024, 1024, 1, 1);
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

void SpriteManager::LoadFont(const char* filename)
{
	//Quite hardcoded for now;

	const int sdf_spacing0[] = {
		32    ,0    ,0    ,0    ,0    ,-3   ,0    ,13,
		33    ,750  ,0    ,15   ,32   ,-3   ,2    ,12,
		34    ,98   ,74   ,22   ,22   ,-3   ,-2   ,16,
		35    ,590  ,43   ,31   ,29   ,-3   ,1    ,24,
		36    ,108  ,0    ,24   ,38   ,-3   ,-2   ,18,
		37    ,582  ,0    ,31   ,33   ,-3   ,1    ,24,
		38    ,72   ,43   ,30   ,31   ,-3   ,2    ,22,
		39    ,1002 ,43   ,15   ,21   ,-3   ,-1   ,9 ,
		40    ,21   ,0    ,20   ,41   ,-3   ,-3   ,14,
		41    ,41   ,0    ,21   ,41   ,-3   ,-3   ,14,
		42    ,23   ,74   ,24   ,23   ,-3   ,-3   ,18,
		43    ,0    ,74   ,23   ,23   ,-3   ,6    ,17,
		44    ,144  ,74   ,17   ,19   ,-3   ,18   ,11,
		45    ,247  ,74   ,24   ,15   ,-3   ,13   ,18,
		46    ,231  ,74   ,16   ,15   ,-3   ,20   ,11,
		47    ,250  ,0    ,29   ,36   ,-3   ,0    ,20,
		48    ,471  ,43   ,28   ,30   ,-3   ,5    ,22,
		49    ,397  ,43   ,26   ,30   ,-3   ,5    ,17,
		50    ,723  ,0    ,27   ,32   ,-3   ,4    ,20,
		51    ,423  ,43   ,24   ,30   ,-3   ,4    ,16,
		52    ,512  ,0    ,22   ,33   ,-3   ,4    ,15,
		53    ,447  ,43   ,24   ,30   ,-3   ,5    ,17,
		54    ,567  ,43   ,23   ,29   ,-3   ,4    ,17,
		55    ,22   ,43   ,22   ,31   ,-3   ,6    ,15,
		56    ,534  ,0    ,24   ,33   ,-3   ,4    ,18,
		57    ,558  ,0    ,24   ,33   ,-3   ,4    ,17,
		58    ,985  ,43   ,17   ,23   ,-3   ,12   ,11,
		59    ,702  ,43   ,17   ,28   ,-3   ,9    ,10,
		60    ,765  ,0    ,25   ,32   ,-3   ,0    ,17,
		61    ,120  ,74   ,24   ,21   ,-3   ,7    ,18,
		62    ,363  ,0    ,24   ,35   ,-3   ,-1   ,16,
		63    ,44   ,43   ,28   ,31   ,-3   ,2    ,21,
		64    ,279  ,0    ,37   ,36   ,-3   ,-1   ,31,
		65    ,413  ,0    ,28   ,33   ,-3   ,1    ,20,
		66    ,102  ,43   ,28   ,30   ,-3   ,3    ,21,
		67    ,613  ,0    ,26   ,32   ,-3   ,2    ,19,
		68    ,130  ,43   ,30   ,30   ,-3   ,3    ,23,
		69    ,160  ,43   ,27   ,30   ,-3   ,3    ,19,
		70    ,639  ,0    ,28   ,32   ,-3   ,2    ,18,
		71    ,790  ,0    ,29   ,31   ,-3   ,2    ,23,
		72    ,819  ,0    ,28   ,31   ,-3   ,2    ,21,
		73    ,187  ,43   ,14   ,30   ,-3   ,3    ,9 ,
		74    ,847  ,0    ,28   ,31   ,-3   ,2    ,19,
		75    ,667  ,0    ,27   ,32   ,-3   ,2    ,19,
		76    ,875  ,0    ,22   ,31   ,-3   ,2    ,16,
		77    ,201  ,43   ,36   ,30   ,-3   ,3    ,27,
		78    ,237  ,43   ,28   ,30   ,-3   ,3    ,21,
		79    ,621  ,43   ,29   ,28   ,-3   ,4    ,22,
		80    ,265  ,43   ,25   ,30   ,-3   ,3    ,17,
		81    ,694  ,0    ,29   ,32   ,-3   ,4    ,22,
		82    ,290  ,43   ,30   ,30   ,-3   ,3    ,20,
		83    ,897  ,0    ,23   ,31   ,-3   ,2    ,18,
		84    ,920  ,0    ,25   ,31   ,-3   ,2    ,16,
		85    ,320  ,43   ,26   ,30   ,-3   ,3    ,19,
		86    ,945  ,0    ,25   ,31   ,-3   ,2    ,16,
		87    ,970  ,0    ,34   ,31   ,-3   ,2    ,25,
		88    ,346  ,43   ,28   ,30   ,-3   ,3    ,21,
		89    ,387  ,0    ,26   ,34   ,-3   ,1    ,19,
		90    ,650  ,43   ,29   ,28   ,-3   ,4    ,22,
		91    ,62   ,0    ,19   ,41   ,-3   ,-3   ,13,
		92    ,208  ,0    ,27   ,37   ,-3   ,0    ,19,
		93    ,0    ,0    ,21   ,43   ,-3   ,-4   ,15,
		94    ,161  ,74   ,24   ,18   ,-3   ,-4   ,18,
		95    ,271  ,74   ,27   ,13   ,-3   ,24   ,21,
		96    ,185  ,74   ,18   ,16   ,-3   ,-1   ,12,
		97    ,47   ,74   ,27   ,22   ,-3   ,11   ,17,
		98    ,525  ,43   ,21   ,29   ,-3   ,4    ,16,
		99    ,854  ,43   ,22   ,25   ,-3   ,9    ,15,
		100   ,546  ,43   ,21   ,29   ,-3   ,5    ,16,
		101   ,742  ,43   ,20   ,26   ,-3   ,8    ,14,
		102   ,316  ,0    ,20   ,35   ,-3   ,3    ,12,
		103   ,336  ,0    ,27   ,35   ,-3   ,5    ,18,
		104   ,441  ,0    ,22   ,33   ,-3   ,1    ,16,
		105   ,235  ,0    ,15   ,36   ,-3   ,0    ,9 ,
		106   ,81   ,0    ,27   ,40   ,-3   ,2    ,9 ,
		107   ,463  ,0    ,23   ,33   ,-3   ,3    ,16,
		108   ,1004 ,0    ,13   ,31   ,-3   ,4    ,8 ,
		109   ,929  ,43   ,32   ,23   ,-3   ,10   ,27,
		110   ,762  ,43   ,24   ,26   ,-3   ,9    ,17,
		111   ,74   ,74   ,24   ,22   ,-3   ,11   ,17,
		112   ,0    ,43   ,22   ,31   ,-3   ,9    ,15,
		113   ,374  ,43   ,23   ,30   ,-3   ,10   ,15,
		114   ,786  ,43   ,21   ,26   ,-3   ,7    ,14,
		115   ,807  ,43   ,21   ,26   ,-3   ,8    ,14,
		116   ,679  ,43   ,23   ,28   ,-3   ,7    ,15,
		117   ,876  ,43   ,24   ,24   ,-3   ,8    ,16,
		118   ,719  ,43   ,23   ,27   ,-3   ,7    ,14,
		119   ,900  ,43   ,29   ,24   ,-3   ,9    ,21,
		120   ,961  ,43   ,24   ,23   ,-3   ,10   ,17,
		121   ,486  ,0    ,26   ,33   ,-3   ,8    ,18,
		122   ,828  ,43   ,26   ,26   ,-3   ,8    ,19,
		123   ,154  ,0    ,25   ,37   ,-3   ,0    ,21,
		124   ,499  ,43   ,26   ,30   ,-3   ,3    ,19,
		125   ,179  ,0    ,29   ,37   ,-3   ,-2   ,24,
		126   ,203  ,74   ,28   ,16   ,-3   ,10   ,22,
	};

	const int sdf_spacing1[] = {
		32    ,0    ,0    ,0    ,0    ,-4   ,0    ,15,
		33    ,733  ,0    ,17   ,38   ,-4   ,2    ,13,
		34    ,442  ,85   ,28   ,26   ,-4   ,-1   ,22,
		35    ,631  ,49   ,36   ,35   ,-4   ,0    ,30,
		36    ,219  ,0    ,28   ,43   ,-4   ,-1   ,23,
		37    ,778  ,0    ,35   ,38   ,-4   ,2    ,29,
		38    ,209  ,49   ,34   ,36   ,-4   ,3    ,29,
		39    ,470  ,85   ,18   ,25   ,-4   ,0    ,12,
		40    ,81   ,0    ,23   ,47   ,-4   ,-3   ,17,
		41    ,104  ,0    ,23   ,47   ,-4   ,-2   ,18,
		42    ,414  ,85   ,28   ,27   ,-4   ,-2   ,23,
		43    ,386  ,85   ,28   ,27   ,-4   ,7    ,22,
		44    ,516  ,85   ,21   ,23   ,-4   ,19   ,14,
		45    ,637  ,85   ,28   ,16   ,-4   ,16   ,22,
		46    ,619  ,85   ,18   ,17   ,-4   ,22   ,15,
		47    ,380  ,0    ,34   ,40   ,-4   ,1    ,26,
		48    ,598  ,49   ,33   ,35   ,-4   ,6    ,26,
		49    ,837  ,49   ,30   ,34   ,-4   ,7    ,20,
		50    ,121  ,49   ,31   ,36   ,-4   ,5    ,25,
		51    ,152  ,49   ,27   ,36   ,-4   ,4    ,21,
		52    ,650  ,0    ,26   ,38   ,-4   ,5    ,19,
		53    ,543  ,49   ,28   ,35   ,-4   ,6    ,21,
		54    ,867  ,49   ,28   ,34   ,-4   ,5    ,22,
		55    ,571  ,49   ,27   ,35   ,-4   ,7    ,18,
		56    ,676  ,0    ,28   ,38   ,-4   ,4    ,22,
		57    ,704  ,0    ,29   ,38   ,-4   ,4    ,21,
		58    ,254  ,85   ,19   ,28   ,-4   ,12   ,14,
		59    ,895  ,49   ,21   ,34   ,-4   ,9    ,15,
		60    ,750  ,0    ,28   ,38   ,-4   ,0    ,22,
		61    ,488  ,85   ,28   ,25   ,-4   ,7    ,22,
		62    ,353  ,0    ,27   ,40   ,-4   ,0    ,20,
		63    ,904  ,0    ,32   ,37   ,-4   ,2    ,25,
		64    ,414  ,0    ,43   ,40   ,-4   ,0    ,36,
		65    ,457  ,0    ,32   ,39   ,-4   ,1    ,24,
		66    ,936  ,0    ,32   ,36   ,-4   ,3    ,26,
		67    ,517  ,0    ,30   ,38   ,-4   ,2    ,23,
		68    ,968  ,0    ,36   ,36   ,-4   ,3    ,28,
		69    ,243  ,49   ,31   ,35   ,-4   ,3    ,23,
		70    ,547  ,0    ,31   ,38   ,-4   ,2    ,23,
		71    ,834  ,0    ,35   ,37   ,-4   ,2    ,27,
		72    ,274  ,49   ,33   ,35   ,-4   ,3    ,25,
		73    ,1004 ,0    ,16   ,35   ,-4   ,3    ,12,
		74    ,869  ,0    ,35   ,37   ,-4   ,2    ,23,
		75    ,0    ,49   ,33   ,36   ,-4   ,3    ,25,
		76    ,307  ,49   ,26   ,35   ,-4   ,4    ,19,
		77    ,667  ,49   ,41   ,34   ,-4   ,4    ,33,
		78    ,708  ,49   ,32   ,34   ,-4   ,4    ,25,
		79    ,916  ,49   ,34   ,33   ,-4   ,4    ,27,
		80    ,333  ,49   ,31   ,35   ,-4   ,3    ,21,
		81    ,33   ,49   ,35   ,36   ,-4   ,5    ,25,
		82    ,364  ,49   ,34   ,35   ,-4   ,3    ,25,
		83    ,68   ,49   ,28   ,36   ,-4   ,3    ,21,
		84    ,398  ,49   ,29   ,35   ,-4   ,4    ,20,
		85    ,427  ,49   ,33   ,35   ,-4   ,3    ,24,
		86    ,460  ,49   ,29   ,35   ,-4   ,4    ,20,
		87    ,740  ,49   ,39   ,34   ,-4   ,4    ,31,
		88    ,779  ,49   ,32   ,34   ,-4   ,4    ,25,
		89    ,279  ,0    ,32   ,40   ,-4   ,1    ,24,
		90    ,978  ,49   ,33   ,32   ,-4   ,6    ,26,
		91    ,25   ,0    ,23   ,48   ,-4   ,-4   ,17,
		92    ,188  ,0    ,31   ,43   ,-4   ,0    ,24,
		93    ,0    ,0    ,25   ,49   ,-4   ,-4   ,20,
		94    ,537  ,85   ,28   ,21   ,-4   ,-3   ,22,
		95    ,665  ,85   ,31   ,15   ,-4   ,26   ,26,
		96    ,598  ,85   ,21   ,18   ,-4   ,1    ,16,
		97    ,273  ,85   ,30   ,27   ,-4   ,12   ,22,
		98    ,811  ,49   ,26   ,34   ,-4   ,5    ,19,
		99    ,132  ,85   ,26   ,29   ,-4   ,10   ,18,
		100   ,96   ,49   ,25   ,36   ,-4   ,4    ,18,
		101   ,0    ,85   ,23   ,31   ,-4   ,8    ,17,
		102   ,311  ,0    ,23   ,40   ,-4   ,4    ,15,
		103   ,247  ,0    ,32   ,42   ,-4   ,4    ,23,
		104   ,489  ,0    ,28   ,39   ,-4   ,1    ,19,
		105   ,334  ,0    ,19   ,40   ,-4   ,1    ,11,
		106   ,48   ,0    ,33   ,47   ,-4   ,1    ,10,
		107   ,578  ,0    ,27   ,38   ,-4   ,3    ,20,
		108   ,605  ,0    ,15   ,38   ,-4   ,3    ,10,
		109   ,183  ,85   ,38   ,28   ,-4   ,10   ,33,
		110   ,49   ,85   ,28   ,30   ,-4   ,10   ,22,
		111   ,303  ,85   ,28   ,27   ,-4   ,11   ,21,
		112   ,489  ,49   ,26   ,35   ,-4   ,10   ,18,
		113   ,515  ,49   ,28   ,35   ,-4   ,11   ,18,
		114   ,158  ,85   ,25   ,29   ,-4   ,9    ,17,
		115   ,77   ,85   ,24   ,30   ,-4   ,9    ,17,
		116   ,950  ,49   ,28   ,33   ,-4   ,7    ,18,
		117   ,331  ,85   ,27   ,27   ,-4   ,10   ,21,
		118   ,23   ,85   ,26   ,31   ,-4   ,8    ,17,
		119   ,221  ,85   ,33   ,28   ,-4   ,10   ,25,
		120   ,358  ,85   ,28   ,27   ,-4   ,12   ,21,
		121   ,620  ,0    ,30   ,38   ,-4   ,9    ,21,
		122   ,101  ,85   ,31   ,30   ,-4   ,9    ,23,
		123   ,160  ,0    ,28   ,43   ,-4   ,0    ,26,
		124   ,179  ,49   ,30   ,36   ,-4   ,3    ,23,
		125   ,127  ,0    ,33   ,44   ,-4   ,-3   ,30,
		126   ,565  ,85   ,33   ,19   ,-4   ,11   ,28,
	};

	mFilenames[mSheets] = filename;
	memcpy(&mGlyphsData[mFonts], sdf_spacing0, sizeof(sdf_spacing0));

	mFonts++;
	memcpy(&mGlyphsData[mFonts], sdf_spacing1, sizeof(sdf_spacing1));


	mFonts++;
	mSheets++;
}

void SpriteManager::DrawSprite(SpriteSheetCode sheetID, int spriteID, vec2f pos, vec2f scale, vec4f tint, vec2f linearFill, float radialFill)
{
	mSpriteInstanceData[mSprites].pointpos = pos;
	mSpriteInstanceData[mSprites].sheetID = sheetID;
	mSpriteInstanceData[mSprites].spriteID = spriteID;
	mSpriteInstanceData[mSprites].size = scale;
	mSpriteInstanceData[mSprites].tint = tint;
	mSpriteInstanceData[mSprites].linearFill = linearFill;
	mSpriteInstanceData[mSprites].radialFill = radialFill;
	
	mSprites++;
}

void SpriteManager::DrawTextSprite(FontCode fontID, float fontSize, vec2f pos, vec4f tint, Alignment align, char* fmt, ...)
{
	char buf[100];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 100, fmt, args);

	vec2f scale(fontSize / 32.0f, fontSize / 32.0f);
	DrawGlyphs(fontID, scale, buf, pos, tint, align);
}

void SpriteManager::DrawGlyphs(FontCode fontID, vec2f scale, char* phrase, vec2f pos, vec4f tint, Alignment align)
{
	char* charPos = phrase;

	if (align != ALIGN_LEFT)
	{
		float width = 0;
		//Precalculate text size
		while (*charPos != '\0')
		{
			Glyph& g = mGlyphsData[fontID][*charPos - 32];
			width += (g.xadvance/1.0f) * scale.x;
			charPos++;
		}

		if (align == ALIGN_CENTER)
			pos.x -= width / 2;
		else if (align == ALIGN_RIGHT)
			pos.x -= width;
	}

	charPos = phrase;
	while(*charPos != '\0')
	{
		if (*charPos >= 32 || *charPos <= 126) {
			Glyph& g = mGlyphsData[fontID][*charPos - 32];
			DrawGlyph(fontID, scale, g, pos + vec2f(g.xoffset / 1.0f, g.yoffset / 1.0f) * scale, tint);
			pos += vec2f(g.xadvance / 1.0f, 0) * scale;
		}
		charPos++;
	}
}

void SpriteManager::DrawGlyph(FontCode fontID, vec2f scale, Glyph& glyph, vec2f pos, vec4f tint)
{
	mGlyphInstanceData[mGlyphs].pointpos = pos;
	mGlyphInstanceData[mGlyphs].sheetID = fontID;
	mGlyphInstanceData[mGlyphs].scale = scale;
	mGlyphInstanceData[mGlyphs].tint = tint;
	mGlyphInstanceData[mGlyphs].size = vec2f((float)glyph.width, (float)glyph.height);
	mGlyphInstanceData[mGlyphs].minUV = vec2f(glyph.x / 1024.0f, glyph.y / 1024.0f);
	mGlyphInstanceData[mGlyphs].maxUV = vec2f((glyph.x + glyph.width) / 1024.0f, (glyph.y + glyph.height) / 1024.0f);
	mGlyphs++;
}