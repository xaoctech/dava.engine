#include "TextureConverter.h"
#include "DXTConverter.h"
#include "PVRConverter.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
	FilePath TextureConverter::ConvertTexture(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, bool updateAfterConversion)
	{
		FilePath outputPath;
		const String& outExtension = GPUFamilyDescriptor::GetCompressedFileExtension(gpuFamily, (DAVA::PixelFormat)descriptor.compression[gpuFamily].format);
		if(outExtension == ".pvr")
		{
			Logger::Info("Starting PVR conversion (%s)...",
							   GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor.compression[gpuFamily].format));
			
			outputPath = PVRConverter::Instance()->ConvertPngToPvr(descriptor, gpuFamily);
		}
		else if(outExtension == ".dds")
		{
			DAVA::Logger::Info("Starting DXT conversion (%s)...",
							   GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(descriptor.compression[gpuFamily].format));
			
			
			if(descriptor.IsCubeMap())
			{
				outputPath = DXTConverter::ConvertCubemapPngToDxt(descriptor, gpuFamily);
			}
			else
			{
				outputPath = DXTConverter::ConvertPngToDxt(descriptor, gpuFamily);
			}
		}
		else
		{
			DVASSERT(false);
		}

		if(updateAfterConversion)
		{
			bool wasUpdated = descriptor.UpdateCrcForFormat(gpuFamily);
			if(wasUpdated)
			{
				descriptor.Save();
			}
		}
		
		return outputPath;
	}
	
	bool TextureConverter::CleanupOldTextures(const DAVA::TextureDescriptor *descriptor,
											  const DAVA::eGPUFamily forGPU,
											  const DAVA::PixelFormat format)
	{
		bool result = true;
		const String & extension = GPUFamilyDescriptor::GetCompressedFileExtension(forGPU, format);
		if(extension == ".pvr")
		{
			DeleteOldPVRTextureIfPowerVr_IOS(descriptor, forGPU);
		}
		else if(extension == ".dds")
		{
			DeleteOldDXTTextureIfTegra(descriptor, forGPU);
		}
		else
		{
			DVASSERT(false);
			result = false;
		}
		
		return result;
	}
	
	void TextureConverter::DeleteOldPVRTextureIfPowerVr_IOS(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
	{
		if(!descriptor || gpu != GPU_POWERVR_IOS) return;
		
		FilePath oldPvrPath = FilePath::CreateWithNewExtension(descriptor->pathname, ".pvr");
		FileSystem::Instance()->DeleteFile(oldPvrPath);
	}
	
	void TextureConverter::DeleteOldDXTTextureIfTegra(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
	{
		if(!descriptor || gpu != GPU_TEGRA) return;
		
		FilePath oldDdsPath = FilePath::CreateWithNewExtension(descriptor->pathname, ".dds");
		FileSystem::Instance()->DeleteFile(oldDdsPath);
	}
	
	FilePath TextureConverter::GetOutputPath(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
	{
		return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
	}
};