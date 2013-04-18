#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA 
{

class Image;
class File;

class LibDxtHelper
{
public:
	
	static bool IsDxtFile(const FilePath & fileName);
	static bool IsDxtFile(File * file);

	//input data only in RGBA8888
	static bool WriteDdsFile(const FilePath & fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps);

	static bool ReadDxtFile(const FilePath & fileName, Vector<Image*> &imageSet);
	static bool ReadDxtFile(File * file, Vector<Image*> &imageSet);

	static bool DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation = false);

	static PixelFormat GetPixelFormat(const FilePath & fileName);
	static PixelFormat GetPixelFormat(File * file);
	
	static bool GetTextureSize(const FilePath & fileName, uint32 & width, uint32 & height);
	static bool GetTextureSize(File * file, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(const FilePath & fileName);
	static uint32 GetMipMapLevelsCount(File * file);

	static uint32 GetDataSize(const FilePath & fileName);
	static uint32 GetDataSize(File * file);
	
private:
	//input data only in RGBA8888
	static bool WriteDxtFile(const FilePath & fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps);
	static bool WriteAtcFile(const FilePath & fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps);
};

};

#endif // __DAVAENGINE_DXT_HELPER_H__