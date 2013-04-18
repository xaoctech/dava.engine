#include "DXTConverter.h"

using namespace DAVA;

String DXTConverter::ConvertPngToDxt(const String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	Vector<Image*> inputImages = ImageLoader::CreateFromFile(fileToConvert);
	if(inputImages.size() == 1)
	{
		Image* image = inputImages[0];
		String outputName = FileSystem::ReplaceExtension(fileToConvert, ".dds");

		if((descriptor.dxtCompression.compressToWidth != 0) && (descriptor.dxtCompression.compressToHeight != 0))
		{
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
			image->ResizeImage(descriptor.dxtCompression.compressToWidth, descriptor.dxtCompression.compressToHeight);
		}
		
		if(LibDxtHelper::WriteDdsFile(outputName,
                                      image->width, image->height, image->data,
                                      descriptor.dxtCompression.format,
                                      (descriptor.generateMipMaps == TextureDescriptor::OPTION_ENABLED)))
		{
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            return outputName;
		}
	}
    
    Logger::Error("[DXTConverter::ConvertPngToDxt] can't convert %s to DXT", fileToConvert.c_str());

    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return String("");
}

String DXTConverter::GetDXTOutput(const DAVA::String &inputDXT)
{
	return FileSystem::ReplaceExtension(inputDXT, ".dds");
}
