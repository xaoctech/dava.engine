#include "Render/LibDxtHelper.h"

#include "Render/Image.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"

#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

#include "Render/ImageLoader.h"
#include "Utils/StringFormat.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"



using namespace nvtt;

namespace DAVA
{

#define DX10HEADER_SIZE 138
#define FORMAT_NAMES_MAP_COUNT 7

class NvttHelper
{
public:
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

	struct PairNvttPixelGLFormat
    {
		static const int32  WRONG_GL_VALUE = -1;
        nvtt::Format nvttFormat;
		PixelFormat  davaFormat;
        int32		 glFormat;
        
        PairNvttPixelGLFormat(nvtt::Format _nvttFormat, PixelFormat _davaFormat, uint32 _glFormat)
        {
            nvttFormat = _nvttFormat;
            davaFormat = _davaFormat;
			glFormat   = _glFormat  ;
        }
    };
	
	const static PairNvttPixelGLFormat formatNamesMap[];

	static bool InitDecompressor(nvtt::Decompressor & dec, const char *fileName);
	
	static bool InitDecompressor(nvtt::Decompressor & dec, File * file);
	
	static bool InitDecompressor(nvtt::Decompressor & dec, const uint8 * mem, uint32 size);
	
	static bool ReadDxtFile(nvtt::Decompressor & dec, Vector<Image*> &imageSet, bool forseSoftwareConvertation);
	
	static PixelFormat GetPixelFormat(nvtt::Decompressor & dec);
	
	static bool GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height);
	
	static uint32 GetMipMapLevelsCount(nvtt::Decompressor & dec);
	
	static uint32 GetDataSize(nvtt::Decompressor & dec);
	
	static bool GetInfo(nvtt::Decompressor & dec, DDSInfo &info);
	
	static void SwapBRChannels(uint8* data, uint32 size);

	static PixelFormat GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat);

	static nvtt::Format GetNVTTFormatByPixelFormat(PixelFormat pixelFormat);
};

const NvttHelper::PairNvttPixelGLFormat NvttHelper::formatNamesMap[] =
{
	//FORMAT_NAMES_MAP_COUNT should be inceased in case of addition to set
	#ifndef __DAVAENGINE_IPHONE__
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
	#else	
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	#endif
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_RGBA,	FORMAT_RGBA8888, PairNvttPixelGLFormat::WRONG_GL_VALUE),
};

PixelFormat NvttHelper::GetPixelFormatByNVTTFormat(nvtt::Format nvttFormat)
{
	PixelFormat retValue = FORMAT_INVALID;
	for(uint32 i = 0; i < FORMAT_NAMES_MAP_COUNT; ++i)
	{
		if(formatNamesMap[i].nvttFormat == nvttFormat)
		{
			 retValue = formatNamesMap[i].davaFormat;
			 break;
		}
	}
	return retValue;
}

nvtt::Format NvttHelper::GetNVTTFormatByPixelFormat(PixelFormat pixelFormat)
{
	//bc5 is unsupported, used to determinate fail in search
	nvtt::Format retValue = Format_BC5;
	for(uint32 i = 0; i < FORMAT_NAMES_MAP_COUNT; ++i)
	{
		if(formatNamesMap[i].davaFormat == pixelFormat)
		{
			retValue = formatNamesMap[i].nvttFormat;
			break;
		}
	}
	return retValue;
}

bool LibDxtHelper::ReadDxtFile(const char *fileName, Vector<Image*> &imageSet)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return false;
	}
	
	return NvttHelper::ReadDxtFile(dec, imageSet, false);
}

bool LibDxtHelper::ReadDxtFile(File * file, Vector<Image*> &imageSet)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return false;
	}
	return NvttHelper::ReadDxtFile(dec, imageSet, false);
}

bool LibDxtHelper::DecompressImageToRGBA(const Image & image, Vector<Image*> &imageSet, bool forseSoftwareConvertation)
{
	if(!(image.format >= FORMAT_DXT1 && image.format <= FORMAT_DXT5NM) )
	{
		Logger::Error("Wrong copression format.");
		return false;
	}
	
		
	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, image.width, image.height);
	
	inputOptions.setMipmapGeneration(false);

	
	CompressionOptions compressionOptions;
	//bc5 is unsupported, used to determinate fail in search
	nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(image.format);
	if(nvtt::Format_BC5 == innerComprFormat)
	{
		return false;
	}
	if(FORMAT_DXT1NM == image.format)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == image.format)
	{
		inputOptions.setNormalMap(true);
	}
	
	compressionOptions.setFormat(innerComprFormat);

	uint32 headerSize = DX10HEADER_SIZE;
	uint8* imageHeaderBuffer = new uint8[headerSize];

	uint32 realHeaderSize = nvtt::Decompressor::getHeader(imageHeaderBuffer, headerSize, inputOptions, compressionOptions);

	nvtt::Decompressor dec;

	uint8* compressedImageBuffer = new uint8[realHeaderSize + image.dataSize];
	memcpy(compressedImageBuffer, imageHeaderBuffer, realHeaderSize);
	memcpy(compressedImageBuffer + realHeaderSize, image.data, image.dataSize);
    SafeDeleteArray(imageHeaderBuffer);
	
	bool retValue = NvttHelper::InitDecompressor(dec, compressedImageBuffer, realHeaderSize + image.dataSize);
	if(retValue)
	{
		retValue = NvttHelper::ReadDxtFile(dec, imageSet, forseSoftwareConvertation);
	}

    SafeDeleteArray(compressedImageBuffer);
	return retValue;
}

bool NvttHelper::ReadDxtFile(nvtt::Decompressor & dec, Vector<Image*> &imageSet, bool forseSoftwareConvertation)
{
    for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
	imageSet.clear();
		
	uint32 width = 0;
	uint32 height = 0;
	uint32 size = 0;
	uint32 mipmapsCount = 0;
	uint32 hSize=0;
	if(!dec.getInfo(mipmapsCount, width, height, size, hSize))
	{
		Logger::Error("Error during header reading.");
		return false;
	}

	if(0 == width || 0 == height || 0 == mipmapsCount)
	{
		Logger::Error("Wrong mipmapsCount/width/height value in dds header.");
		return false;
	}
	

	nvtt::Format format;
	if(!dec.getCompressionFormat(format))
	{
		Logger::Error("Getting format information cause error.");
		return false;
	}

	//check hardware support, in case of rgb use nvtt to reorder bytes
	if( (!forseSoftwareConvertation) &&
		RenderManager::Instance()->GetCaps().isDXTSupported &&
		format != Format_RGB )
	{
		bool retValue = false;
		uint8* compressedImges = new uint8[size];
	
		if(!dec.getRawData(compressedImges, size))
		{
			Logger::Error("Reading compressed data cause error in nvtt lib.");

            SafeDeleteArray(compressedImges);
			return retValue;
		}

        retValue = true;
        
        uint32 offset = 0;
		for(uint32 i = 0; i < mipmapsCount; ++i)
		{
			uint32 mipSize = 0;
            if(!dec.getMipmapSize(i, mipSize))
            {
                retValue = false;
                break;
            }
            
			uint8* concreteImageStartPointer = compressedImges + offset;
            
			PixelFormat pixFormat = GetPixelFormat(dec);
			Image* innerImage = Image::Create(width, height, pixFormat);
			memcpy(innerImage->data, concreteImageStartPointer, mipSize);
			imageSet.push_back(innerImage);

            height = Max((uint32)1, height / 2);
            width = Max((uint32)1, width / 2);
            offset += mipSize;
		}

        SafeDeleteArray(compressedImges);
		return retValue;
	}
	else
	{
		for(uint32 i = 0; i < mipmapsCount; ++i)
		{
			Image* innerImage = Image::Create(width, height, FORMAT_RGBA8888);
			Memset(innerImage->data, 0, innerImage->dataSize);
            
			if(dec.process(innerImage->data, innerImage->dataSize, i))
			{
				SwapBRChannels(innerImage->data, innerImage->dataSize);
				imageSet.push_back(innerImage);
                
//               ImageLoader::Save(innerImage, Format("C://dev//layer_%d.png", i));
//				ImageLoader::Save(innerImage, Format("/Levels/layer_%d.png", i));
			}
			else
			{
				Logger::Error("nvtt lib compression fail.");
				SafeRelease(innerImage);
				return false;
			}
			
            height = Max((uint32)1, height / 2);
            width = Max((uint32)1, width / 2);
		}
	}

	return true;
}

struct ErrorHandlerDXT: ErrorHandler
{
    virtual void error(Error e)
    {
        Logger::Error("[AA] %s", nvtt::errorString(e));
    }
};
    
bool LibDxtHelper::WriteDxtFile(const String & fileNameOriginal, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps)
{
	//creating tmp dds file, nvtt accept only filename.dds as input, because of this the last letter befor "." should be changed to "_".
	String fileName = fileNameOriginal;
	FileSystem * fs =FileSystem::Instance();
	String extension = fs->GetExtension(fileNameOriginal);
	size_t index = 0;
	index = fileName.find(extension, index);
	index --; // set pointer to last char befor .
    if (index == String::npos || extension != ".dds") 
	{
		Logger::Error("Wrong input file name.");
		return false;
	}
    // Make the replacement. 
    fileName.replace(index, 1, "_");

	if (fileName.empty())
	{
		return false;
	}

	if(!( (compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM)|| (compressionFormat == FORMAT_RGBA8888)) )
	{
		Logger::Error("Wrong copression format.");
		return false;
	}
	
	uint32 imgDataSize = width * height * Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
	NvttHelper::SwapBRChannels(data, imgDataSize);

	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
    inputOptions.setMipmapGeneration(generateMipmaps);
    inputOptions.setMipmapData(data, width, height);

	CompressionOptions compressionOptions;

	nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);
	//bc5 is unsupported, used to determinate fail in search
	if(nvtt::Format_BC5 == innerComprFormat)
	{
		return false;
	}
	if(FORMAT_DXT1NM == compressionFormat)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == compressionFormat)
	{
		inputOptions.setNormalMap(true);
	}
	
	compressionOptions.setFormat(innerComprFormat);
    
	OutputOptions outputOptions;
	outputOptions.setFileName(fileName.c_str());
    ErrorHandlerDXT errorHandler;
    outputOptions.setErrorHandler(&errorHandler);
	
	Compressor compressor;
	bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
 
	if(!ret)
	{
		Logger::Error("Error during writing DDS file");
	}

	if(fs->IsFile(fileNameOriginal))
	{
		fs->DeleteFile(fileNameOriginal);
	}
	
	if(!fs->MoveFile(fileName, fileNameOriginal))
	{
		Logger::Error("Temporary dds file renamig failed.");
	}

	return ret;
}

bool LibDxtHelper::IsDxtFile(const char *filePathname)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(filePathname);
}

bool LibDxtHelper::IsDxtFile(File * file)
{
	nvtt::Decompressor dec;
	return NvttHelper::InitDecompressor(dec,file);
}

PixelFormat LibDxtHelper::GetPixelFormat(const char* fileName)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

PixelFormat LibDxtHelper::GetPixelFormat(File * file)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

PixelFormat NvttHelper::GetPixelFormat(nvtt::Decompressor & dec)
{
	bool res = false;
	nvtt::Format innerFormat;

	res = dec.getCompressionFormat(innerFormat);

	if(!res)
	{
		Logger::Error("Wrong dds file compression format.");
		return FORMAT_INVALID;
	}

	PixelFormat retValue = NvttHelper::GetPixelFormatByNVTTFormat(innerFormat);
	

	return retValue;
}

uint32 LibDxtHelper::GetMipMapLevelsCount(const char *fileName)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(File * file)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 NvttHelper::GetMipMapLevelsCount(nvtt::Decompressor & dec)
{
	DDSInfo info;
	GetInfo(dec, info);
	return info.mipMupsNumber;
}

uint32 LibDxtHelper::GetDataSize(const char *fileName)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return NvttHelper::GetDataSize(dec);
}

uint32 LibDxtHelper::GetDataSize(File * file)
{
		nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return 0;
	}

	return NvttHelper::GetDataSize(dec);
}

uint32 NvttHelper::GetDataSize(nvtt::Decompressor & dec)
{
	DDSInfo info;
	GetInfo(dec, info);
	return info.dataSize;
}

bool LibDxtHelper::GetTextureSize(const char *fileName, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return false;
	}

	return NvttHelper::GetTextureSize(dec, width, height);
}

bool LibDxtHelper::GetTextureSize(File * file, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec ;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return false;
	}

	return NvttHelper::GetTextureSize(dec, width, height);
}

bool NvttHelper::GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height)
{
	DDSInfo info;
	
	bool ret = GetInfo(dec, info);
	width = info.width;
	height = info.height;
	return ret;
}

bool NvttHelper::GetInfo(nvtt::Decompressor & dec, DDSInfo &info)
{
	bool retVal = false;
	
	if(dec.getInfo(info.mipMupsNumber, info.width, info.height, info.dataSize, info.headerSize))
	{
		retVal = true;
	}
	else
	{
		Logger::Error("Error: can't read info from DDS file.");
	}
		
	return retVal;
}

void NvttHelper::SwapBRChannels(uint8* data, uint32 size)
{
	for(uint32 i = 0; i < size; i+=4)
	{
		//RGBA <-> BGRA
	
		uint8* rComponent = data + i;
		
		uint8* bComponent = data + i + 2;
		uint8 tmp = *rComponent;
		*rComponent = *bComponent;
		*bComponent = tmp;
	}
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, const char *fileName)
{
	if(NULL == fileName )
	{
		Logger::Error("Wrong fileName.");
		return false;
	}

	if(!dec.initWithDDSFile(fileName))
	{
		Logger::Error("Wrong file.");
		return false;
	}
	return true;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, File * file)
{
	if(NULL == file)
	{
		Logger::Error("Wrong handler.");
		return false;
	}
	file->Seek(0, File::SEEK_FROM_START);
	uint32 fileSize = file->GetSize();
	uint8* fileBuffer= new uint8[fileSize];
	file->Read(fileBuffer, fileSize);
    bool initied = dec.initWithDDSFile(fileBuffer, fileSize);
	file->Seek(0, File::SEEK_FROM_START);
	SafeDeleteArray(fileBuffer);
    return initied;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, const uint8 * mem, uint32 size)
{
	if(NULL == mem || size == 0 )
	{
		Logger::Error("Wrong buffer.");
		return false;
	}

	if(!dec.initWithDDSFile(mem, size))
	{
		Logger::Error("Wrong buffer.");
		return false;
	}
	return true;
}

};
