#include "Render/LibDxtHelper.h"

#include "Render/Image.h"
#include "Render/Texture.h"
#include "Render/RenderManager.h"

#include <libdxt/nvtt.h>
#include <libdxt/nvtt_extra.h>

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

#include "Utils/Utils.h"


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
		uint32 mipmapsCount;
		uint32 headerSize;
		
		DDSInfo()
		{
			width		  = 0;
			height		  = 0;
			dataSize	  = 0;
			mipmapsCount = 0;
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
			glFormat   = _glFormat;
        }
    };
	
	const static PairNvttPixelGLFormat formatNamesMap[];

	static bool InitDecompressor(nvtt::Decompressor & dec, const FilePath & fileName);
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
#if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 GL_COMPRESSED_RGB_S3TC_DXT1_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1,	FORMAT_DXT1NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 GL_COMPRESSED_RGBA_S3TC_DXT1_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT1a,	FORMAT_DXT1A,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT1_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT3,	FORMAT_DXT3,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT

#if defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT),
#else
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5,	FORMAT_DXT5,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
	NvttHelper::PairNvttPixelGLFormat(nvtt::Format_DXT5n,	FORMAT_DXT5NM,	 PairNvttPixelGLFormat::WRONG_GL_VALUE),
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

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

bool LibDxtHelper::ReadDxtFile(const FilePath &fileName, Vector<Image*> &imageSet)
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
		Logger::Error("[LibDxtHelper::DecompressImageToRGBA] Wrong copression format (%d).", image.format);
		return false;
	}
	nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(image.format);
	if(nvtt::Format_BC5 == innerComprFormat)
	{ 	//bc5 is unsupported, used to determinate fail in search
		Logger::Error("[LibDxtHelper::DecompressImageToRGBA] Can't work with nvtt::Format_BC5.");
		return false;
	}
	
		
	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, image.width, image.height);
	inputOptions.setMipmapGeneration(false);

	CompressionOptions compressionOptions;
	compressionOptions.setFormat(innerComprFormat);
	if(FORMAT_DXT1NM == image.format)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == image.format)
	{
		inputOptions.setNormalMap(true);
	}
	
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
		
    DDSInfo info;
    if(!GetInfo(dec, info))
    {
		Logger::Error("[NvttHelper::ReadDxtFile] Error during header reading.");
		return false;
    }

	if(0 == info.width || 0 == info.height || 0 == info.mipmapsCount)
	{
		Logger::Error("[NvttHelper::ReadDxtFile] Wrong mipmapsCount/width/height value in dds header.");
		return false;
	}
	

	nvtt::Format format;
	if(!dec.getCompressionFormat(format))
	{
		Logger::Error("[NvttHelper::ReadDxtFile] Getting format information cause error.");
		return false;
	}

	//check hardware support, in case of rgb use nvtt to reorder bytes
	if(!forseSoftwareConvertation && RenderManager::Instance()->GetCaps().isDXTSupported)
	{
		uint8* compressedImges = new uint8[info.dataSize];
	
		if(!dec.getRawData(compressedImges, info.dataSize))
		{
			Logger::Error("[NvttHelper::ReadDxtFile] Reading compressed data cause error in nvtt lib.");

            SafeDeleteArray(compressedImges);
			return false;
		}
        
        if(format == Format_RGB)
        {
            SwapBRChannels(compressedImges, info.dataSize);
        }

        PixelFormat pixFormat = GetPixelFormat(dec);

		bool retValue = true;
        
        uint32 offset = 0;
		for(uint32 i = 0; i < info.mipmapsCount; ++i)
		{
			uint32 mipSize = 0;
            if(!dec.getMipmapSize(i, mipSize))
            {
                retValue = false;
                break;
            }
            
			Image* innerImage = Image::Create(info.width, info.height, pixFormat);
			memcpy(innerImage->data, compressedImges + offset, mipSize);
			imageSet.push_back(innerImage);

            info.height = Max((uint32)1, info.height / 2);
            info.width = Max((uint32)1, info.width / 2);

            offset += mipSize;
		}

        SafeDeleteArray(compressedImges);
		return retValue;
	}
	else
	{
#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
        Logger::Error("[NvttHelper::ReadDxtFile] Android should have hardware decoding of DDS. iPhone should have no support of DDS.");
		return false;
#else //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
        
		for(uint32 i = 0; i < info.mipmapsCount; ++i)
		{
			Image* innerImage = Image::Create(info.width, info.height, FORMAT_RGBA8888);
			if(dec.process(innerImage->data, innerImage->dataSize, i))
			{
				SwapBRChannels(innerImage->data, innerImage->dataSize);
				imageSet.push_back(innerImage);
			}
			else
			{
				Logger::Error("nvtt lib compression fail.");
				SafeRelease(innerImage);
				return false;
			}
			
            info.height = Max((uint32)1, info.height / 2);
            info.width = Max((uint32)1, info.width / 2);
		}
#endif //#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)

	}

	return true;
}


bool LibDxtHelper::WriteDxtFile(const FilePath & fileNameOriginal, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, bool generateMipmaps)
{
	if(!( (compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM)|| (compressionFormat == FORMAT_RGBA8888)) )
	{
		Logger::Error("[LibDxtHelper::WriteDxtFile] Wrong copression format (%d).", compressionFormat);
		return false;
	}

	//creating tmp dds file, nvtt accept only filename.dds as input, because of this the last letter befor "." should be changed to "_".
	String extension = fileNameOriginal.GetExtension();
    if(0 != CompareCaseInsensitive(extension, ".dds"))
    {
		Logger::Error("[LibDxtHelper::WriteDxtFile] Wrong input file name (%s).", fileNameOriginal.GetAbsolutePathname().c_str());
        return false;
    }
    

    nvtt::Format innerComprFormat = NvttHelper::GetNVTTFormatByPixelFormat(compressionFormat);
    if(nvtt::Format_BC5 == innerComprFormat)
	{ 	//bc5 is unsupported, used to determinate fail in search
		Logger::Error("[LibDxtHelper::WriteDxtFile] Can't work with nvtt::Format_BC5.");
		return false;
	}
    
	uint32 imgDataSize = width * height * Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
	NvttHelper::SwapBRChannels(data, imgDataSize);

	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
    inputOptions.setMipmapGeneration(generateMipmaps);
    inputOptions.setMipmapData(data, width, height);


	CompressionOptions compressionOptions;
	compressionOptions.setFormat(innerComprFormat);
	if(FORMAT_DXT1NM == compressionFormat)
	{
		compressionOptions.setColorWeights(1, 1, 0);
	}
	else if (FORMAT_DXT5NM == compressionFormat)
	{
		inputOptions.setNormalMap(true);
	}
	
    
	OutputOptions outputOptions;
	FilePath fileName = FilePath::CreateWithNewExtension(fileNameOriginal, "_dds");
	outputOptions.setFileName(fileName.GetAbsolutePathname().c_str());
	
	Compressor compressor;
	bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
    if(ret)
    {
        FileSystem::Instance()->DeleteFile(fileNameOriginal);
        if(!FileSystem::Instance()->MoveFile(fileName, fileNameOriginal, true))
        {
            Logger::Error("[LibDxtHelper::WriteDxtFile] Temporary dds file renamig failed.");
            ret = false;
        }
    }
    else
    {
		Logger::Error("[LibDxtHelper::WriteDxtFile] Error during writing DDS file (%s).", fileName.GetAbsolutePathname().c_str());
    }
    
	return ret;
}

bool LibDxtHelper::IsDxtFile(const FilePath & filePathname)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(filePathname.GetAbsolutePathname().c_str());
}

bool LibDxtHelper::IsDxtFile(File * file)
{
	nvtt::Decompressor dec;
	return NvttHelper::InitDecompressor(dec,file);
}

PixelFormat LibDxtHelper::GetPixelFormat(const FilePath & fileName)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

PixelFormat LibDxtHelper::GetPixelFormat(File * file)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return FORMAT_INVALID;
	}

	return NvttHelper::GetPixelFormat(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(const FilePath & fileName)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(File * file)
{
	nvtt::Decompressor dec;

	if(!NvttHelper::InitDecompressor(dec, file))
	{
		return 0;
	}

	return NvttHelper::GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetDataSize(const FilePath & fileName)
{
	nvtt::Decompressor dec;

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


bool LibDxtHelper::GetTextureSize(const FilePath & fileName, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec;

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
    
PixelFormat NvttHelper::GetPixelFormat(nvtt::Decompressor & dec)
{
    nvtt::Format innerFormat;
    
    if(!dec.getCompressionFormat(innerFormat))
    {
        Logger::Error("[NvttHelper::GetPixelFormat] Wrong dds file compression format.");
        return FORMAT_INVALID;
    }
    
    return NvttHelper::GetPixelFormatByNVTTFormat(innerFormat);
}

uint32 NvttHelper::GetMipMapLevelsCount(nvtt::Decompressor & dec)
{
    DDSInfo info;
    GetInfo(dec, info);
    return info.mipmapsCount;
}
    
uint32 NvttHelper::GetDataSize(nvtt::Decompressor & dec)
{
    DDSInfo info;
    GetInfo(dec, info);
    return info.dataSize;
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
	bool retVal = dec.getInfo(info.mipmapsCount, info.width, info.height, info.dataSize, info.headerSize);
	if(!retVal)
	{
		Logger::Error("[NvttHelper::GetInfo] Error: can't read info from DDS file.");
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

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, const FilePath & fileName)
{
	if(!fileName.IsInitalized())
	{
		Logger::Error("[NvttHelper::InitDecompressor] try init with empty name");
		return false;
	}

	if(!dec.initWithDDSFile(fileName.GetAbsolutePathname().c_str()))
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong dds file (%s).", fileName.GetAbsolutePathname().c_str());
		return false;
	}
    
	return true;
}

bool NvttHelper::InitDecompressor(nvtt::Decompressor & dec, File * file)
{
	if(NULL == file)
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong handler.");
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
		Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer params.");
		return false;
	}

	if(!dec.initWithDDSFile(mem, size))
	{
		Logger::Error("[NvttHelper::InitDecompressor] Wrong buffer.");
		return false;
	}
	return true;
}

};
