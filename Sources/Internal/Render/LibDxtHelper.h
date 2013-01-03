#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

namespace DAVA 
{

class Texture;
class Sprite;
class Image;
class File;

class LibDxtHelper
{
public:
	
	static bool IsDxtFile(const char *fileName);

	static bool IsDxtFile(File * file);

	//input data only in RGBA8888
	static bool WriteDxtFile(const String & fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps);

	static bool ReadDxtFile(const char *fileName, Vector<Image*> &imageSet);
	
	static bool ReadDxtFile(File * file, Vector<Image*> &imageSet);

	static bool DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation = false);

	static PixelFormat GetPixelFormat(const char* fileName);
	
	static PixelFormat GetPixelFormat(File * file);
	
	static bool GetTextureSize(const char *fileName, uint32 & width, uint32 & height);

	static bool GetTextureSize(File * file, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(const char *fileName);

	static uint32 GetMipMapLevelsCount(File * file);

	static uint32 GetDataSize(const char *fileName);
	
	static uint32 GetDataSize(File * file);

	static void test();
};

};

#endif // __DAVAENGINE_DXT_HELPER_H__