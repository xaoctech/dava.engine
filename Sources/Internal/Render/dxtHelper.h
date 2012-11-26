#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/Image.h"
#include <nvtt/nvtt.h>

namespace DAVA 
{

class Texture;
class Sprite;
class Image;

class DxtWrapper
{
public:
    
	static bool IsDxtFile(File *file) {return false;};
    
	static int ReadDxtFile(const char *file, Image * image){return 0;};
	static int ReadDxtFile(File *infile, Image * image){return 0;};
	static void WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat format);

};
/*
class PngImage : public BaseObject
{
public:
	PngImage();
	~PngImage();
	
	bool Create(int32 _width, int32 _height);
	bool CreateFromFBOSprite(Sprite * fboSprite);
	
	bool Load(const char * filename);
	bool Save(const char * filename);
	
	void DrawImage(int sx, int sy, PngImage * image);
	void DrawRect(const Rect2i & rect, uint32 color);

	uint8 * GetData() { return data; };
	int32 GetWidth() { return width; };
	int32 GetHeight() { return height; }; 
private:	
	int32		width;
	int32		height;
	uint8  *	data;
    PixelFormat format;		
};*/
};

#endif // __DXT_HELPER_H__