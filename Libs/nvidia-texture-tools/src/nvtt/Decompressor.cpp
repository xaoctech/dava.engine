// Copyright NVIDIA Corporation 2008 -- Ignacio Castano <icastano@nvidia.com>
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

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
	m_dds = new nv::DirectDrawSurface(pathToDDSFile);
	
	if (!m_dds->isValid())
	{
		printf("The file '%s' is not a valid DDS file.\n", pathToDDSFile);
		return false;
	}
	
	return true;
}

//NVTT_API void erase();
void Decompressor::erase()
{
	m.erase();
}

void Decompressor::Private::erase()
{
	if(NULL != m_dds)
	{
		delete m_dds;
	}
}

//NVTT_API bool getDecompressedSize(unsigned int mipmapNumber, unsigned int * size) const;
bool Decompressor::getDecompressedSize(unsigned int * width, unsigned int * height) const
{
	return m.getDecompressedSize(width, height);
}

bool Decompressor::Private::getDecompressedSize(unsigned int * width, unsigned int * height) const
{
	if(NULL == width || NULL == height || NULL == m_dds)
	{
		return false;
	}

	*width = m_dds->width();
	*height= m_dds->height();
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
