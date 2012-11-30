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

PixelFormat DxtWrapper::GetPixelFormat(const char* filePathname)
{
	PixelFormat retValue = FORMAT_INVALID;

	nvtt::Decompressor dec;
	bool res = false;
	nvtt::Format innerFormat;

	res = dec.initWithDDSFile(filePathname);
	if(!res)
	{
		return FORMAT_INVALID;
	}

	res = dec.getCompressionFormat(&innerFormat);
	//dec.erase();

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

bool DxtWrapper::WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat)
{
	if(NULL == fileName)
		return false;

	if( !(compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5) )
		return false;

	printf("* Writing DXT(*.DDS) file (%d x %d): %s\n", width, height, fileName);

	
	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
	inputOptions.setMipmapGeneration(false);
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
		break;
	case FORMAT_RGBA8888:
		innerComprFormat = nvtt::Format_RGBA;
		break;
	default:
		return false;
	}

	compressionOptions.setFormat(innerComprFormat);
	
	Compressor compressor;
	compressor.process(inputOptions, compressionOptions, outputOptions);
	
	return true;
}

bool DxtWrapper::IsDxtFile(const char *filePathname)
{
	nvtt::Decompressor dec;
	bool res = false;

	res = dec.initWithDDSFile(filePathname);
	//dec.erase();

	return res;
}

uint32 DxtWrapper::GetMipMapLevelsCount(const char *fileName)
{
	nvtt::Decompressor dec;
	uint32 number = 0;

	if(dec.initWithDDSFile(fileName))
	{
		if(!dec.getMipMapCount(&number))
		{
			number = 0;
		}
	}

	//dec.erase();
	return number;
}

Image * DxtWrapper::ReadDxtFile(const char *fileName)
{
	Image * retVal = NULL;
	if(NULL == fileName )
	{
		return retVal;
	}
	nvtt::Decompressor dec;

	if(dec.initWithDDSFile(fileName))
	{
		uint32 width = 0;
		uint32 height = 0;
		if(dec.getDecompressedSize(&width, &height))
		{
			if(0 != width || 0 != height)
			{
				Image* innerImage = Image::Create(width, height, FORMAT_RGBA8888);
				uint32 size = width * height * 4;
				if(dec.process(innerImage->data, size))
				{
					retVal = innerImage;
				}
				else
				{
					SafeRelease(innerImage);
				}
			}
		}
	}

	//dec.erase();
	return retVal;
}

bool DxtWrapper::getDecompressedSize(const char *fileName, uint32 * width, uint32 * height)
{
	bool retVal = false;
	nvtt::Decompressor dec;
	if(dec.initWithDDSFile(fileName))
	{
		if(dec.getDecompressedSize(width, height))
		{
			retVal = true;
		}
	}


	//dec.erase();
	return retVal;
}

void DxtWrapper::Test()
{
	const char* fnamePng = "C:\\dds\\1\\3.png";
	const char* fnameDds = "C:\\dds\\1\\3.dds";
	bool res = false;
	
	//Image * img = new Image();
	//int pngInt = LibPngWrapper::ReadPngFile(fnamePng, img);
	//
	//uint8* imgData = img->data;
	//uint32 imgDataSize = img->width * img->height *4;
	//for(uint32 i = 0; i < imgDataSize; i+=4)
	//{
	//	//RGBA -> BGRA
	//	uint8* rComponent = imgData + i;
	//	
	//	uint8* bComponent = imgData + i + 2;
	//	uint8 tmp = *rComponent;
	//	*rComponent = *bComponent;
	//	*bComponent = tmp;
	//}
	Texture* pngTex = Texture::CreateFromFile(fnamePng);
	Sprite* pngSprite = Sprite::CreateFromTexture(pngTex, 0, 0, (float32)pngTex->width, (float32)pngTex->height);
	Image * img  = CreateImageAsBGRA8888(pngSprite);
	res = WriteDxtFile(fnameDds, img->width, img->height, img->data, PixelFormat::FORMAT_DXT1);

	res = IsDxtFile(fnameDds);
	
	uint32 mapCount = GetMipMapLevelsCount(fnameDds);

	Image * retImg = ReadDxtFile(fnameDds);
	LibPngWrapper::WritePngFile("C:\\dds\\1\\1_out.png",retImg->width,retImg->height,retImg->data,PixelFormat::FORMAT_RGBA8888);

	PixelFormat retFormat = GetPixelFormat(fnameDds);

	uint32 w = 0;
	uint32 h = 0;
	res = getDecompressedSize(fnameDds, &w, &h);
}


Image * DxtWrapper::CreateImageAsBGRA8888(Sprite *sprite)
{

    Sprite *renderTarget = Sprite::CreateAsRenderTarget(sprite->GetWidth(), sprite->GetHeight(), FORMAT_RGBA8888);
    RenderManager::Instance()->SetRenderTarget(renderTarget);
    
    sprite->SetPosition(0, 0);
    sprite->Draw();
    
    RenderManager::Instance()->RestoreRenderTarget();
    
    Texture *renderTargetTexture = renderTarget->GetTexture();
    Image *resultImage = renderTargetTexture->CreateImageFromMemory();
    
	uint8* imgData = resultImage->data;
	uint32 imgDataSize = resultImage->width * resultImage->height *4;
	for(uint32 i = 0; i < imgDataSize; i+=4)
	{
		//RGBA -> BGRA
	
		uint8* rComponent = imgData + i;
		
		uint8* bComponent = imgData + i + 2;
		uint8 tmp = *rComponent;
		*rComponent = *bComponent;
		*bComponent = tmp;
	}
    SafeRelease(renderTarget);
    return resultImage;
}
