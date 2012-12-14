#include "Render/dxtHelper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "Render/RenderManager.h"
#include "Render/2D/Sprite.h"
#include "Render/Texture.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "FileSystem/FileSystem.h"
#include "Render/LibPngHelpers.h"

using namespace DAVA;
using namespace nvtt;

void DxtWrapper::Test()
{
	const char *fileName = "c:\\dds\\1\\nm_mips.dds";
	nvtt::Decompressor dec;
	if(!dec.initWithDDSFile(fileName))
	{
		return ;
	}

	uint32 width = 0;
	uint32 height = 0;
	uint32 size = 0;
	uint32 mipmapNumber = 0;
	uint32 headerSize = 0;
	if(!dec.getInfo(mipmapNumber, width, height, size,headerSize))
	{
		return ;
	}
	//
	uint8* compressedImg = new uint8(size );
	if(!dec.getRawData(compressedImg, size))
	{
		return;
	}
	
	std::vector<uint32> vec;

	for(uint32 i =0; i < mipmapNumber; ++i)
	{
		uint32 size= 0;
		if(dec.getMipmapSize(i, size))
		{
			vec.push_back(size);
		}
	}

	GLuint texid;  

	return;
	glGenTextures( 1, &texid );
	glBindTexture ( GL_TEXTURE_2D, texid );
	glCompressedTexImage2D ( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, width, height, 0, size, compressedImg);
	switch(glGetError())
	{
		case GL_INVALID_ENUM:
			Logger::Instance()->Debug("GL_INVALID_ENUM");
			break;
			
		case GL_INVALID_VALUE:
			Logger::Instance()->Debug("GL_INVALID_VALUE");
			break;
			
		case GL_INVALID_OPERATION:
			Logger::Instance()->Debug("GL_INVALID_OPERATION");
			break;
			
		case GL_NO_ERROR:
			Logger::Instance()->Debug("GL_NO_ERROR");
			break;
			
		default:
			Logger::Instance()->Debug("other value");
	}
	//

	
	int32 uncompressed_width = 0;
	int32 uncompressed_heigth = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &uncompressed_width);
	switch(glGetError())
	{
		case GL_INVALID_ENUM:
			Logger::Instance()->Debug("GL_INVALID_ENUM");
			break;
			
		case GL_INVALID_VALUE:
			Logger::Instance()->Debug("GL_INVALID_VALUE");
			break;
			
		case GL_INVALID_OPERATION:
			Logger::Instance()->Debug("GL_INVALID_OPERATION");
			break;
			
		case GL_NO_ERROR:
			Logger::Instance()->Debug("GL_NO_ERROR");
			break;
			
		default:
			Logger::Instance()->Debug("other value");
	}

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &uncompressed_heigth);
	switch(glGetError())
	{
		case GL_INVALID_ENUM:
			Logger::Instance()->Debug("GL_INVALID_ENUM");
			break;
			
		case GL_INVALID_VALUE:
			Logger::Instance()->Debug("GL_INVALID_VALUE");
			break;
			
		case GL_INVALID_OPERATION:
			Logger::Instance()->Debug("GL_INVALID_OPERATION");
			break;
			
		case GL_NO_ERROR:
			Logger::Instance()->Debug("GL_NO_ERROR");
			break;
			
		default:
			Logger::Instance()->Debug("other value");
	}

	Image* innerImage = Image::Create(uncompressed_width, uncompressed_heigth, FORMAT_RGBA8888);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, innerImage->GetData());
	switch(glGetError())
	{
		case GL_INVALID_ENUM:
			Logger::Instance()->Debug("GL_INVALID_ENUM");
			break;
			
		case GL_INVALID_VALUE:
			Logger::Instance()->Debug("GL_INVALID_VALUE");
			break;
			
		case GL_INVALID_OPERATION:
			Logger::Instance()->Debug("GL_INVALID_OPERATION");
			break;
			
		case GL_NO_ERROR:
			Logger::Instance()->Debug("GL_NO_ERROR");
			break;
			
		default:
			Logger::Instance()->Debug("other value");
	}

	ImageLoader::Save(innerImage, "c:\\dds\\1\\nm_13_12_12.png");
}

bool DxtWrapper::ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet)
{
	for(uint32 i = 0; i < imageSet.size(); ++i)
	{
		SafeRelease(imageSet[i]);
	}

	
	if(NULL == fileName )
	{
		return false;
	}

	nvtt::Decompressor dec;
	if(!dec.initWithDDSFile(fileName))
	{
		return false;
	}
	
	//uint32 mipmapNumber = GetMipMapLevelsCount(fileName);
	//
	//if(0 == mipmapNumber)
	//{
	//	return false;
	//}
	
	imageSet.clear();

	uint32 width = 0;
	uint32 height = 0;
	uint32 size = 0;
	uint32 mipmapNumber = 0;
	uint32 hSize=0;
	if(!dec.getInfo(mipmapNumber, width, height, size, hSize))
	{
		return false;
	}
	
	if(0 == width || 0 == height || 0 == mipmapNumber)
	{
		return false;
	}
	

	for(uint32 i = 0; i < mipmapNumber; ++i)
	{
		// change size to support mipmaps sizes
		if(i != 0)
		{
			height /= 2;
			width /= 2;
		}

		height = height > 1 ? height : 0;
		width = width > 1 ? width : 0;
		if(height == 0 || width ==0)
		{
			// final mipmap is built
			break;
		}

		Image* innerImage = Image::Create(width, height, FORMAT_RGBA8888);
		uint32 size = width * height * 4;
		if(dec.process(innerImage->data, size, i))
		{
			ConvertFromBGRAtoRGBA(innerImage->data, size);
			imageSet.push_back(innerImage);
		}
		else
		{
			SafeRelease(innerImage);
			return false;
		}
	}
	
	return true;
}

bool DxtWrapper::WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmupNumber)
{
	if (NULL == fileName)
		return false;

	if( !(compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM || compressionFormat == FORMAT_RGBA8888) )
		return false;
	
	uint32 imgDataSize = width * height *4;

	ConvertFromBGRAtoRGBA(data, imgDataSize);

	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
	if(mipmupNumber > 0)
	{
		inputOptions.setMipmapGeneration(true, mipmupNumber);
	}
	else
	{
		inputOptions.setMipmapGeneration(false);
	}
	inputOptions.setMipmapData(data, width, height);

	OutputOptions outputOptions;
	outputOptions.setFileName(fileName);
	CompressionOptions compressionOptions;

	nvtt::Format innerComprFormat;
	switch (compressionFormat)
	{
	case FORMAT_DXT1:
		innerComprFormat = nvtt::Format_DXT1;
		break;
	case FORMAT_DXT1NM:
		innerComprFormat = nvtt::Format_DXT1;
		compressionOptions.setColorWeights(1, 1, 0);
		break;
	case FORMAT_DXT1A:
		innerComprFormat = nvtt::Format_DXT1a;
		break;
	case FORMAT_DXT3:
		innerComprFormat = nvtt::Format_DXT3;
		break;
	case FORMAT_DXT5:
		innerComprFormat = nvtt::Format_DXT5;
		break;
	case FORMAT_DXT5NM:
		innerComprFormat = nvtt::Format_DXT5n;
		inputOptions.setNormalMap(true);
		break;
	case FORMAT_RGBA8888:
		innerComprFormat = nvtt::Format_RGBA;
		break;
	default:
		return false;
	}

	compressionOptions.setFormat(innerComprFormat);
	
	Compressor compressor;
	bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
	if(!ret)
	{
		Logger::Debug("Error during writing DDS file");
	}
	return ret;
}

bool DxtWrapper::IsDxtFile(const char *filePathname)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(filePathname);
}

PixelFormat DxtWrapper::GetPixelFormat(const char* filePathname)
{
	PixelFormat retValue = FORMAT_INVALID;

	nvtt::Decompressor dec;
	bool res = false;
	nvtt::Format innerFormat;

	res = dec.initWithDDSFile(filePathname);
	if(!res)
	{
		Logger::Debug("Can't init from file");
		return FORMAT_INVALID;
	}

	res = dec.getCompressionFormat(innerFormat);

	if(!res)
	{
		return FORMAT_INVALID;
	}

	switch (innerFormat)
	{
	case Format_RGB:
		retValue = FORMAT_RGBA8888;
		break;
	case Format_DXT1:
		retValue = FORMAT_DXT1;
		break;
	case Format_DXT3:
		retValue = FORMAT_DXT3;
		break;
	case Format_DXT5:
		retValue = FORMAT_DXT5;
		break;
	case Format_DXT5n:
		retValue = FORMAT_DXT5NM;
		break;
	case Format_BC4:
	case Format_BC5:
	default:
		retValue = FORMAT_INVALID;
		break;
	}

	return retValue;
}

uint32 DxtWrapper::GetMipMapLevelsCount(const char *fileName)
{
	DDSInfo info;
	GetInfo(fileName, info);
	return info.mipMupsNumber;
}

uint32 DxtWrapper::GetDataSize(const char *fileName)
{
	DDSInfo info;
	GetInfo(fileName, info);
	return info.dataSize;
}

bool DxtWrapper::GetTextureSize(const char *fileName, uint32 & width, uint32 & height)
{
	DDSInfo info;
	
	bool ret = GetInfo(fileName, info);
	width = info.width;
	height = info.height;
	return ret;
}

bool DxtWrapper::GetInfo(const char *fileName, DDSInfo &info)
{
	bool retVal = false;
	if(NULL == fileName )
	{
		return retVal;
	}

	nvtt::Decompressor dec;
	if(dec.initWithDDSFile(fileName))
	{
		if(dec.getInfo(info.mipMupsNumber, info.width, info.height, info.dataSize, info.headerSize))
		{
			retVal = true;
		}
		else
		{
			Logger::Debug("Error: can't read info from DDS file.");
		}
	}
	else
	{
		Logger::Debug("Can't init from file.");
	}

	return retVal;
}

Image * DxtWrapper::CreateImageAsRGBA8888(Sprite *sprite)
{
    Sprite *renderTarget = Sprite::CreateAsRenderTarget(sprite->GetWidth(), sprite->GetHeight(), FORMAT_RGBA8888);
    RenderManager::Instance()->SetRenderTarget(renderTarget);

    sprite->SetPosition(0, 0);
    sprite->Draw();
    
    RenderManager::Instance()->RestoreRenderTarget();

    Texture *renderTargetTexture = renderTarget->GetTexture();
    Image *resultImage = renderTargetTexture->CreateImageFromMemory();

    SafeRelease(renderTarget);
    return resultImage;
}

void DxtWrapper::ConvertFromBGRAtoRGBA(uint8* data, uint32 size)
{
	for(uint32 i = 0; i < size; i+=4)
	{
		//RGBA -> BGRA
	
		uint8* rComponent = data + i;
		
		uint8* bComponent = data + i + 2;
		uint8 tmp = *rComponent;
		*rComponent = *bComponent;
		*bComponent = tmp;
	}
}

