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
    
	static bool IsDxtFile(const char *fileName);

	//input data only in RGBA8888
	static bool WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmapLevelNumber);

	static uint32 GetMipMapLevelsCount(const char *fileName);

	static bool ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet);

	static PixelFormat GetPixelFormat(const char* fileName);

	static bool getDecompressedSize(const char *fileName, uint32 * width,  uint32 * height);

	static void Test();

private:

	
	
	static Image * CreateImageAsRGBA8888(Sprite *sprite);

	static void ConvertFromBGRAtoRGBA(uint8* data, uint32 size);
};

};

#endif // __DXT_HELPER_H__