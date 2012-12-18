#include "Render/LibDxtHelper.h"

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

#include <nvtt/nvtt.h>
#include <nvtt/nvtt_extra.h>

using namespace DAVA;
using namespace nvtt;

void LibDxtHelper::Test()
{
	const char *fileName = "c:\\dds\\2\\3rgb.dds";
	const char *fileNameWrite = "c:\\dds\\2\\3_n.dds";
	Vector<DAVA::Image*> imageSet;
	ReadDxtFile(fileName, imageSet);
	
	Image* img = imageSet.front();
	
	WriteDxtFile(fileNameWrite, img->width, img->height, img->data,FORMAT_DXT1,5);

	Vector<DAVA::Image*> imageSetN;
	ReadDxtFile(fileNameWrite, imageSetN);
	for(uint32 i = 0 ; i < 5; i++)
	{
		ImageLoader::Save(imageSetN[i], "c:\\dds\\2\\3__n_p.png");
	}
	
	
	
}

bool LibDxtHelper::ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, fileName))
	{
		return false;
	}
	
	return ReadDxtFile(dec, imageSet);
}

bool LibDxtHelper::ReadDxtFile(FILE * file, Vector<DAVA::Image*> &imageSet)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, file))
	{
		return false;
	}
	return ReadDxtFile(dec, imageSet);
}

bool LibDxtHelper::ReadDxtFile(nvtt::Decompressor & dec, Vector<DAVA::Image*> &imageSet)
{
	for(uint32 i = 0; i < imageSet.size(); ++i)
	{
		SafeRelease(imageSet[i]);
	}
	imageSet.clear();
		
	uint32 width = 0;
	uint32 height = 0;
	uint32 size = 0;
	uint32 mipmapNumber = 0;
	uint32 hSize=0;
	if(!dec.getInfo(mipmapNumber, width, height, size, hSize))
	{
		DAVA::Logger::Error("Error during header reading.");
		return false;
	}
	
	if(0 == width || 0 == height || 0 == mipmapNumber)
	{
		DAVA::Logger::Error("Wrong mipmapNumber/width/height value in dds header.");
		return false;
	}
	

	nvtt::Format format;
	if(!dec.getCompressionFormat(format))
	{
		DAVA::Logger::Error("Getting format information cause error.");
		return false;
	}

	//check hardware support, in case of rgb use nvtt to reorder bytes
	if(RenderManager::Instance()->GetCaps().isDXTSupported && (format != Format_RGB) )
	{
		bool retValue = false;
		uint8* compressedImges = new uint8[size];
	
		if(!dec.getRawData(compressedImges, size))
		{
			delete[] compressedImges;
			DAVA::Logger::Error("Reading compressed data cause error in nvtt lib.");
			return retValue;
		}

		std::vector<uint32> vecSizes;

		for(uint32 i =0; i < mipmapNumber; ++i)
		{
			uint32 mipSize= 0;
			if(dec.getMipmapSize(i, mipSize))
			{
				vecSizes.push_back(mipSize);
			}
		}

		for(uint32 i = 0; i < mipmapNumber; ++i)
		{
			uint32 concreteWidth = width;
			uint32 concreteHeight = height;

			uint32 offset = 0;
			for (uint32 j = 0; j < i; ++j)
			{
				concreteWidth  /= 2;
				concreteHeight /= 2;

				offset += vecSizes[j];
			}
			if (concreteWidth < 4  || concreteHeight < 4)
			{
				//don't work with size  lower than dxt block
				retValue = true;
				break;
			}
			GLuint texid;  
			glGenTextures( 1, &texid );
			glBindTexture ( GL_TEXTURE_2D, texid );
			
			uint8* concreteImageStartPointer = compressedImges + offset;
			
			uint32 internalFormat = GetGlCompressionFormatByDDSInfo(format);
			if(internalFormat == 0)
			{
				DAVA::Logger::Error("Unknown internal format of dds file.");
				retValue = false;
				break;
			}

			glCompressedTexImage2D ( GL_TEXTURE_2D, i, internalFormat, concreteWidth, concreteHeight, 0, vecSizes[i], concreteImageStartPointer);
			CheckGlError();

			int32 uncompressed_width = 0;
			int32 uncompressed_heigth = 0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_WIDTH, &uncompressed_width);
			CheckGlError();

			glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_HEIGHT, &uncompressed_heigth);
			CheckGlError();

			Image* innerImage = Image::Create(uncompressed_width, uncompressed_heigth, FORMAT_RGBA8888);
			glGetTexImage(GL_TEXTURE_2D, i, GL_RGBA, GL_UNSIGNED_BYTE, innerImage->GetData());
			CheckGlError();
			imageSet.push_back(innerImage);
			//ImageLoader::Save(innerImage, "c:\\dds\\1\\nm_13_12_12.png");
			retValue = true;

		}

		delete[] compressedImges;
		return retValue;
	}
	else
	{
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
				DAVA::Logger::Error("nvtt lib compression fail.");
				SafeRelease(innerImage);
				return false;
			}
		}
	}
	
	
	return true;
}

bool LibDxtHelper::WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmupNumber)
{
	if (NULL == fileName)
	{
		return false;
	}

	if(!( (compressionFormat >= FORMAT_DXT1 && compressionFormat <= FORMAT_DXT5NM)|| (compressionFormat == FORMAT_RGBA8888)) )
	{
		DAVA::Logger::Error("Wrong copression format.");
		return false;
	}
	
	uint32 imgDataSize = width * height *4;

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
		{
			return false;
		}
	}
	compressionOptions.setFormat(innerComprFormat);

	//check hardware support, in case of rgb use nvtt to reorder bytes
	if(RenderManager::Instance()->GetCaps().isDXTSupported && (compressionFormat != FORMAT_RGBA8888) )
	{
		bool retValue = false;
		uint32 dx10HeaderSize = 138;
		uint8* buffer = new uint8[dx10HeaderSize];
		uint32 writeSize = nvtt::Decompressor::getHeader(buffer, dx10HeaderSize, inputOptions, compressionOptions);
		if(writeSize == 0)
		{
			DAVA::Logger::Error("DDS header generation failed.");
			return retValue;
		}
		Image* resizeImg = NULL;
		if(mipmupNumber > 0)
		{
			resizeImg = Image::Create(width, height, FORMAT_RGBA8888);
			memcpy(resizeImg->data, data, resizeImg->dataSize);
		}

		File* file = File::Create(fileName, File::CREATE | File::WRITE);
		if(file)
		{
			file->Write(buffer, writeSize);

			uint32 nW = width;
			uint32 nH = height;
			for(uint32 i = 0; i < mipmupNumber+1 ; ++i)
			{
				
				if(i != 0)
				{
					nW /=2;
					nH /=2;
					nW = nW < 4 ? 4 : nW;
					nH = nH < 4 ? 4 : nH;
					if(nW != resizeImg->width)
					{
						resizeImg->ResizeImage(nW, nH);
					}
					data = resizeImg->data;
				}
				GLuint texId;
				glGenTextures(1, &texId);
				glBindTexture(GL_TEXTURE_2D, texId);
				glTexImage2D(GL_TEXTURE_2D, i, GetGlCompressionFormatByDDSInfo(innerComprFormat), nW, nH, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				CheckGlError();
				int compressed;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED, &compressed);
				CheckGlError();
				if (compressed != GL_TRUE)
				{
					DAVA::Logger::Error("Input data isn't compressed img.");
					retValue =  false;
					break;
				}
				
				int32 compressed_size;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);
				uint8* compressed_texture = new uint8[compressed_size];
				glGetCompressedTexImage(GL_TEXTURE_2D, i, compressed_texture);
				CheckGlError();

				file->Write(compressed_texture, compressed_size);
				delete[] compressed_texture;
				retValue = true;
			}
		}
		if(NULL != resizeImg)
		{
			SafeRelease(resizeImg);
		}

		delete file;
		delete[] buffer;
		
		return retValue;
	}
	else
	{
		ConvertFromBGRAtoRGBA(data, imgDataSize);
		
		
		OutputOptions outputOptions;
		outputOptions.setFileName(fileName);
		
		Compressor compressor;
		bool ret = compressor.process(inputOptions, compressionOptions, outputOptions);
		if(!ret)
		{
			DAVA::Logger::Error("Error during writing DDS file");
		}
		return ret;
	}
	
}

bool LibDxtHelper::IsDxtFile(const char *filePathname)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(filePathname);
}

bool LibDxtHelper::IsDxtFile(FILE * file)
{
	nvtt::Decompressor dec;
	return dec.initWithDDSFile(file);
}

PixelFormat LibDxtHelper::GetPixelFormat(const char* fileName)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, fileName))
	{
		return FORMAT_INVALID;
	}

	return GetPixelFormat(dec);
}

PixelFormat LibDxtHelper::GetPixelFormat(FILE * file)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, file))
	{
		return FORMAT_INVALID;
	}

	return GetPixelFormat(dec);
}

PixelFormat LibDxtHelper::GetPixelFormat(nvtt::Decompressor & dec)
{
	PixelFormat retValue = FORMAT_INVALID;

	bool res = false;
	nvtt::Format innerFormat;

	res = dec.getCompressionFormat(innerFormat);

	if(!res)
	{
		DAVA::Logger::Error("Wrong dds file compression format.");
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

uint32 LibDxtHelper::GetMipMapLevelsCount(const char *fileName)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(FILE * file)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, file))
	{
		return 0;
	}

	return GetMipMapLevelsCount(dec);
}

uint32 LibDxtHelper::GetMipMapLevelsCount(nvtt::Decompressor & dec)
{
	DDSInfo info;
	GetInfo(dec, info);
	return info.mipMupsNumber;
}

uint32 LibDxtHelper::GetDataSize(const char *fileName)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, fileName))
	{
		return 0;
	}

	return GetDataSize(dec);
}

uint32 LibDxtHelper::GetDataSize(FILE * file)
{
		nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, file))
	{
		return 0;
	}

	return GetDataSize(dec);
}

uint32 LibDxtHelper::GetDataSize(nvtt::Decompressor & dec)
{
	DDSInfo info;
	GetInfo(dec, info);
	return info.dataSize;
}

bool LibDxtHelper::GetTextureSize(const char *fileName, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, fileName))
	{
		return false;
	}

	return GetTextureSize(dec, width, height);
}

bool LibDxtHelper::GetTextureSize(FILE * file, uint32 & width, uint32 & height)
{
	nvtt::Decompressor dec ;

	if(!InitDecompressor(dec, file))
	{
		return false;
	}

	return GetTextureSize(dec, width, height);
}

bool LibDxtHelper::GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height)
{
	DDSInfo info;
	
	bool ret = GetInfo(dec, info);
	width = info.width;
	height = info.height;
	return ret;
}

bool LibDxtHelper::GetInfo(nvtt::Decompressor & dec, DDSInfo &info)
{
	bool retVal = false;
	
	if(dec.getInfo(info.mipMupsNumber, info.width, info.height, info.dataSize, info.headerSize))
	{
		retVal = true;
	}
	else
	{
		DAVA::Logger::Error("Error: can't read info from DDS file.");
	}
		
	return retVal;
}

void LibDxtHelper::ConvertFromBGRAtoRGBA(uint8* data, uint32 size)
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

uint32 LibDxtHelper::GetGlCompressionFormatByDDSInfo(nvtt::Format format)
{
	uint32 retValue = 0;

	switch (format)
	{
	case Format_RGB:
		break;
	case Format_DXT1:
		retValue = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case Format_DXT3:
		retValue = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case Format_DXT5:
		retValue = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case Format_DXT5n:
		retValue = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case Format_BC4:
	case Format_BC5:
	default:
		break;
	}

	return retValue;
}

void LibDxtHelper::CheckGlError()
{
	switch(glGetError())
	{
	case GL_INVALID_ENUM:
		DAVA::Logger::Error("DXTHelper: GL_INVALID_ENUM");
		Logger::Instance()->Debug("GL_INVALID_ENUM");
		break;

	case GL_INVALID_VALUE:
		DAVA::Logger::Error("DXTHelper: GL_INVALID_VALUE");
		Logger::Instance()->Debug("GL_INVALID_VALUE");
		break;

	case GL_INVALID_OPERATION:
		DAVA::Logger::Error("DXTHelper: GL_INVALID_OPERATION");
		Logger::Instance()->Debug("GL_INVALID_OPERATION");
		break;

	case GL_NO_ERROR:
		//Logger::Instance()->Debug("GL_NO_ERROR");
		break;

	default:
		DAVA::Logger::Error("DXTHelper: some GL error");
		Logger::Instance()->Debug("other value");
	}
}

bool LibDxtHelper::InitDecompressor(nvtt::Decompressor & dec, const char *fileName)
{
	if(NULL == fileName )
	{
		DAVA::Logger::Error("Wrong fileName.");
		return false;
	}

	if(!dec.initWithDDSFile(fileName))
	{
		DAVA::Logger::Error("Wrong file.");
		return false;
	}
	return true;
}

bool LibDxtHelper::InitDecompressor(nvtt::Decompressor & dec, FILE * file)
{
	if(NULL == file)
	{
		DAVA::Logger::Error("Wrong handler.");
		return false;
	}

	if(!dec.initWithDDSFile(file))
	{
		DAVA::Logger::Error("Wrong file.");
		return false;
	}
	return true;
}