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

		if(descriptor.generateMipMaps > 0)
		{
			uint32 targetSize = image->width > image->height ? image->height : image->width;
			if(!(descriptor.dxtCompression.compressToWidth == 0 && descriptor.dxtCompression.compressToHeight == 0))//if compressToHeight compressToWidth == 0 use size from png
			{
				uint32 ctw = descriptor.dxtCompression.compressToWidth;
				uint32 cth = descriptor.dxtCompression.compressToHeight;
				targetSize = ctw > cth ? cth : ctw;
			}
			uint32 lastSize = targetSize;

			for( ; lastSize >= 2 ; ++mipmupNumber)//count mipmaps number wich cause minimum side size to 2
			{
				lastSize /= 2;
			}
			mipmupNumber--; // nvtt begin counting from 0
		}

		if(!(descriptor.dxtCompression.compressToWidth == 0 && descriptor.dxtCompression.compressToHeight == 0))
		{
			image->ResizeImage(descriptor.dxtCompression.compressToWidth, descriptor.dxtCompression.compressToHeight);
		}
		
		if(!DxtWrapper::WriteDxtFile(outputName.c_str(), image->width, image->height, image->data, 
			descriptor.dxtCompression.format, 0))// dxt helper supports mipmaps but it's unused
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

