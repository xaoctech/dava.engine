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
	dec.erase();

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

bool DxtWrapper::WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmupNumber)
{
	if(NULL == fileName )
		return false;

	if( !(compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM) )
		return false;
	
	uint32 imgDataSize = width * height *4;


	ConvertFromBGRAtoRGBA(data, imgDataSize);

	InputOptions inputOptions;
	inputOptions.setTextureLayout(TextureType_2D, width, height);
	if(mipmupNumber > 0)
	{
		inputOptions.setMipmapGeneration(mipmupNumber);
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
	return ret;
}

bool DxtWrapper::IsDxtFile(const char *filePathname)
{
	nvtt::Decompressor dec;
	bool res = false;

	res = dec.initWithDDSFile(filePathname);
	dec.erase();

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

	dec.erase();
	return number;
}

bool DxtWrapper::ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet)
{
	for(uint32 i = 0; i < imageSet.size(); ++i)
	{
		SafeRelease(imageSet[i]);
	}
	

	bool retVal = false;
	if(NULL == fileName )
	{
		return retVal;
	}
	nvtt::Decompressor dec;

	if(dec.initWithDDSFile(fileName))
	{
		uint32 mipmapNumber = 0;
		if(dec.getMipMapCount(&mipmapNumber))
		{
			imageSet.clear();
			
			uint32 width = 0;
			uint32 height = 0;
			if(dec.getDecompressedSize(&width, &height))
			{
				if(0 == width || 0 == height)
				{
					retVal = false;
				}
				else
				{
					for(uint32 i = 0; i < mipmapNumber; ++i)
					{
						if(i != 0)
						{
							height /= 2;
							width /= 2;
						}
						Image* innerImage = Image::Create(width, height, FORMAT_RGBA8888);
						uint32 size = width * height * 4;
						if(dec.process(innerImage->data, size, i))
						{
							ConvertFromBGRAtoRGBA(innerImage->data, size);
							imageSet.push_back(innerImage);
							retVal = true;
						}
						else
						{
							SafeRelease(innerImage);
							retVal = false;
							break;
						}
					}
				}
			}
		}
	}

	dec.erase();
	return retVal;
}

bool DxtWrapper::getDecompressedSize(const char *fileName, uint32 * width, uint32 * height)
{
	bool retVal = false;
	if(NULL == fileName )
	{
		return retVal;
	}
	nvtt::Decompressor dec;
	if(dec.initWithDDSFile(fileName))
	{
		if(dec.getDecompressedSize(width, height))
		{
			retVal = true;
		}
	}
	dec.erase();
	return retVal;
}

void DxtWrapper::Test()
{/*
	const char* fnamePng = "C:\\dds\\1\\nm.png";
	const char* fnameDds = "C:\\dds\\1\\nm.dds";
	const char* fnameDds1 = "C:\\dds\\1\\nm1.dds";
	bool res = false;
	
	//Image testImg;
	//LibPngWrapper::ReadPngFile(fnamePng, &testImg);

	Texture* pngTex = Texture::CreateFromFile(fnamePng);
	Sprite* pngSprite = Sprite::CreateFromTexture(pngTex, 0, 0, (float32)pngTex->width, (float32)pngTex->height);
	
	Image *imgToSaveTest = pngSprite->GetTexture()->CreateImageFromMemory();   

	Image * img  = CreateImageAsRGBA8888(pngSprite);

	res = WriteDxtFile(fnameDds, img->width, img->height, img->data, PixelFormat::FORMAT_DXT5NM, 0);

	res = IsDxtFile(fnameDds);
	
	uint32 mapCount = GetMipMapLevelsCount(fnameDds);

	Vector<Image *> vec;

	res = ReadDxtFile(fnameDds, vec);
	for(uint32 i = 0; i < vec.size(); ++i)
	{
		LibPngWrapper::WritePngFile("C:\\dds\\1\\nm_out.png",vec[i]->width, vec[i]->height, vec[i]->data, PixelFormat::FORMAT_RGBA8888);
	}
	PixelFormat retFormat = GetPixelFormat(fnameDds);

	uint32 w = 0;
	uint32 h = 0;
	res = getDecompressedSize(fnameDds, &w, &h);*/
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
