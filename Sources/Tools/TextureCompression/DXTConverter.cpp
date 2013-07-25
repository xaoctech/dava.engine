#include "DXTConverter.h"

#include "FileSystem/FilePath.h"
#include "Render/TextureDescriptor.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "Render/LibDxtHelper.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
    
FilePath DXTConverter::ConvertPngToDxt(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
    
    Vector<Image*> inputImages = ImageLoader::CreateFromFile(fileToConvert);
    if(inputImages.size() == 1)
    {
        Image* image = inputImages[0];
        
        FilePath outputName = GetDXTOutput(descriptor, gpuFamily);
        
        if((descriptor.compression[gpuFamily].compressToWidth != 0) && (descriptor.compression[gpuFamily].compressToHeight != 0))
        {
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
            image->ResizeImage(descriptor.compression[gpuFamily].compressToWidth, descriptor.compression[gpuFamily].compressToHeight);
        }
        
        if(LibDxtHelper::WriteDdsFile(outputName,
                                      image->width, image->height, &(image->data), 1,
                                      (PixelFormat) descriptor.compression[gpuFamily].format,
                                      (descriptor.settings.generateMipMaps == TextureDescriptor::OPTION_ENABLED)))
        {
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            return outputName;
        }
    }
    
    Logger::Error("[DXTConverter::ConvertPngToDxt] can't convert %s to DXT", fileToConvert.GetAbsolutePathname().c_str());
    
    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return FilePath();
}
	
FilePath DXTConverter::ConvertCubemapPngToDxt(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
	
	Vector<Image*> inputImages;
	Vector<String> faceNames;
	Texture::GenerateCubeFaceNames(descriptor.pathname.GetAbsolutePathname().c_str(), faceNames);
	for(int i = 0; i < faceNames.size(); ++i)
	{
		Vector<Image*> tempImages = ImageLoader::CreateFromFile(faceNames[i]);
		if(tempImages.size() == 1)
		{
			inputImages.push_back(tempImages[0]);
		}
		
		if(tempImages.size() != 1 ||
		   ((inputImages.size() > 0 && tempImages.size() > 0) &&
			(inputImages[0]->width != tempImages[0]->width || inputImages[0]->height != tempImages[0]->height)))
		{
			for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
			for_each(tempImages.begin(), tempImages.end(), SafeRelease<Image>);
			
			Logger::Error("[DXTConverter::ConvertCubemapPngToDxt] can't convert %s to cubemap DXT", fileToConvert.GetAbsolutePathname().c_str());
			return FilePath();
		}
	}
    
    
    if(inputImages.size() == DAVA::Texture::CUBE_FACE_MAX_COUNT)
    {
        Image* image = inputImages[0];
        
        FilePath outputName = GetDXTOutput(descriptor, gpuFamily);
        
        if((descriptor.compression[gpuFamily].compressToWidth != 0) && (descriptor.compression[gpuFamily].compressToHeight != 0))
        {
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
            image->ResizeImage(descriptor.compression[gpuFamily].compressToWidth, descriptor.compression[gpuFamily].compressToHeight);
        }
		
		uint8** faceData = new uint8*[inputImages.size()];
		for(int i = 0; i < inputImages.size(); ++i)
		{
			faceData[i] = inputImages[i]->data;
		}
        
        if(LibDxtHelper::WriteDdsFile(outputName,
                                      image->width, image->height, faceData, inputImages.size(),
                                      (PixelFormat) descriptor.compression[gpuFamily].format,
                                      (descriptor.settings.generateMipMaps == TextureDescriptor::OPTION_ENABLED)))
        {
			SafeDeleteArray(faceData);
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            return outputName;
        }
		
		SafeDeleteArray(faceData);
    }
    
    Logger::Error("[DXTConverter::ConvertCubemapPngToDxt] can't convert %s to cubemap DXT", fileToConvert.GetAbsolutePathname().c_str());
    
    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return FilePath();	
}

FilePath DXTConverter::GetDXTOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
}

};

