#include <nvtt/nvtt.h>

#include <nvcore/Memory.h>
#include <nvcore/Ptr.h>


#include "Decompressor.h"

#include <nvimage/Image.h>


#include <nvimage/ImageIO.h>
#include <nvcore/StrLib.h>
#include <nvmath/color.h>

using namespace nv;
using namespace nvtt;

Decompressor::Decompressor() : m(*new Decompressor::Private())
{
}

Decompressor::~Decompressor()
{
	m.erase();
	delete &m;
}



//NVTT_API bool initWithDDSFile(const char * pathToDDSFile);
bool Decompressor::initWithDDSFile(const char * pathToDDSFile)
{
	return m.initWithDDSFile(pathToDDSFile);
}

bool Decompressor::Private::initWithDDSFile(const char * pathToDDSFile) 
{
	if(NULL == pathToDDSFile)
	{
		return false;
	}
	nv::DirectDrawSurface dds(pathToDDSFile);
	if (!dds.isValid())
	{
		printf("The file '%s' is not a valid DDS file.\n", pathToDDSFile);
		return false;
	}
	m_dds = new nv::DirectDrawSurface(dds);
	return true;
}

//NVTT_API void erase();
void Decompressor::erase()
{
	//m.erase();
}

void Decompressor::Private::erase()
{
	if(NULL != m_dds)
	{
		delete m_dds;
	}
}

//NVTT_API bool getDecompressedSize(unsigned int mipmapNumber, unsigned int * size) const;
bool Decompressor::getDecompressedSize(unsigned int mipmapNumber, unsigned int * size) const
{
	return m.getDecompressedSize(mipmapNumber, size);
}

bool Decompressor::Private::getDecompressedSize(unsigned int mipmapNumber, unsigned int * size) const
{
	if(NULL == size || NULL == m_dds)
	{
		return false;
	}

	nv::Image img;
	m_dds->mipmap(&img, 0, mipmapNumber); // get first image
	*size = img.width() * img.height() * sizeof(Color32);
	return true;
}
		
//NVTT_API bool process(void * data, unsigned int size, unsigned int mimpmapNumber) const;
bool Decompressor::process(void * data, unsigned int size) const
{
	return m.decompress( data, size);
}

bool Decompressor::Private::decompress(void * data, unsigned int size) const
{
	if(NULL == m_dds || NULL == data || 0 == size)
	{
		return false;
	}

	nv::Image img;
	m_dds->mipmap(&img, 0, 0); 
	
	Color32 * innerContent = img.pixels();
	uint innerSize = img.width() * img.height() ;
	
	uint rawSize = innerSize * sizeof(Color32);

	if(size != rawSize)
	{
		return false;
	}

	if(img.format() == Image::Format_ARGB)
	{
		memcpy(data, innerContent, rawSize);
	}
	else if(img.format() == Image::Format_RGB)
	{
		for(uint i = 0 ; i < innerSize; ++i)
		{
			Color32 * sourcePointer = innerContent + i;
			Color32 * destPointer = (Color32 *)data + i;
			(*destPointer).r = (*sourcePointer).r;
			(*destPointer).g = (*sourcePointer).g;
			(*destPointer).b = (*sourcePointer).b;
			(*destPointer).b = 0xff;
		}
	}
	else
	{
		return false;
	}

	return true;
}

//NVTT_API bool getMipMapCount(unsigned int * mipmapCount) const;
bool Decompressor::getMipMapCount(unsigned int * mipmapCount) const
{
	return m.getMipMapCount(mipmapCount);
}

bool Decompressor::Private::getMipMapCount(unsigned int * mipmapCount) const
{
	if(NULL == mipmapCount || NULL == m_dds)
	{
		return false;
	}
	*mipmapCount = m_dds->mipmapCount();
	return true;
}

bool Decompressor::getCompressionFormat(Format * comprFormat) const
{
	return m.getCompressionFormat(comprFormat);
}

bool Decompressor::Private::getCompressionFormat(Format * comprFormat) const
{
	if(NULL == comprFormat)
		return false;

	return m_dds->getFormat(comprFormat);
}
