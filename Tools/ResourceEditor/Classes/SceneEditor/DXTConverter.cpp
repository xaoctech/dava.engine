#include "DXTConverter.h"
#include "Render/TextureDescriptor.h"
#include "Render/DXTHelper.h"

using namespace DAVA;

DXTConverter::DXTConverter()
{
}

DXTConverter::~DXTConverter()
{
}

String DXTConverter::ConvertPngToDxt(const String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	String outputName;
	Vector<Image*> inputImages = ImageLoader::CreateFromFile(fileToConvert);
	if(inputImages.size() == 1)
	{
		Image* image = inputImages[0];
		outputName = GetDXTToolOutput(fileToConvert);

		uint8 mipmupNumber = 0;
		if(!(descriptor.dxtCompression.compressToWidth == 0 && descriptor.dxtCompression.compressToHeight == 0))
		{
			if(descriptor.generateMipMaps > 0)
			{
				uint32 targetSize = descriptor.dxtCompression.compressToWidth;
				uint32 ratio = image->width / descriptor.dxtCompression.compressToWidth;
				uint32 i = 0;
				for ( ; ratio > 1; ++i)
				{
					ratio = ratio >> 1;
				}
				mipmupNumber = 10 - i;
			}
			image->ResizeImage(descriptor.dxtCompression.compressToWidth, descriptor.dxtCompression.compressToHeight);
		}
		if(!DxtWrapper::WriteDxtFile(outputName.c_str(), image->width, image->height, image->data, 
			descriptor.dxtCompression.format, mipmupNumber))
		{
			outputName.clear();
		}
	}


	return outputName;
}

String DXTConverter::GetDXTToolOutput(const DAVA::String &inputDxt)
{
	return FileSystem::ReplaceExtension(inputDxt, ".dds");
}

