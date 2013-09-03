#ifndef __DAVAENGINE_TOOLS_TEXTURE_CONVERTER_H__
#define __DAVAENGINE_TOOLS_TEXTURE_CONVERTER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
	class TextureDescriptor;
	class TextureConverter
	{
	public:
		
		static FilePath ConvertTexture(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, bool updateAfterConversion);
		static bool CleanupOldTextures(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily forGPU, const DAVA::PixelFormat format);
		static FilePath GetOutputPath(const TextureDescriptor &descriptor, eGPUFamily gpuFamily);
		
		DAVA_DEPRECATED(static void DeleteOldPVRTextureIfPowerVr_IOS(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu));
		DAVA_DEPRECATED(static void DeleteOldDXTTextureIfTegra(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu));
	};
}

#endif /* defined(__DAVAENGINE_TEXTURE_CONVERTER_H__) */
