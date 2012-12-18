#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/Image.h"

namespace nvtt
{
	enum Format;
};

namespace DAVA 
{

class Texture;
class Sprite;
class Image;


class LibDxtHelper
{
public:
	static bool IsDxtFile(const char *fileName);

	//input data only in RGBA8888
	static bool WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmupNumber);

	static bool ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet);

	static PixelFormat GetPixelFormat(const char* fileName);

	static bool GetTextureSize(const char *fileName, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(const char *fileName);

	static uint32 GetDataSize(const char *fileName);

	static void Test();
private:

	struct DDSInfo
	{
		uint32 width;
		uint32 height;
		uint32 dataSize;
		uint32 mipMupsNumber;
		uint32 headerSize;

		DDSInfo()
		{
			width		  = 0;
			height		  = 0;
			dataSize	  = 0;
			mipMupsNumber = 0;
			headerSize	  = 0;
		}
	};
	
	static bool GetInfo(const char *fileName, DDSInfo &info);

	static Image * CreateImageAsRGBA8888(Sprite *sprite);

	static void ConvertFromBGRAtoRGBA(uint8* data, uint32 size);

	static uint32 GetGlCompressionFormatByDDSInfo(nvtt::Format format);

	static void CheckGlError();
};

};

#endif // __DXT_HELPER_H__