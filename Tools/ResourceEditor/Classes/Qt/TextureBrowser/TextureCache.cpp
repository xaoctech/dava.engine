/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "TextureCache.h"
#include "TextureConvertor.h"
#include "Main/mainwindow.h"
#include "ImageTools/ImageTools.h"
#include "CubemapEditor/CubemapUtils.h"

#include <QPainter>
#include <QFileInfo>

TextureCache::TextureCache()
{
    curThumbnailWeight = 0;
	curOriginalWeight = 0;
    for(int i = 0 ; i < DAVA::GPU_FAMILY_COUNT; ++i)
    {
        curConvertedWeight[i] = 0;
    }
    
    new TextureConvertor();
    
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyThumbnail(const DAVA::TextureDescriptor *, const DAVA::Vector<QImage>&)), this, SLOT(ReadyThumbnail(const DAVA::TextureDescriptor *, const DAVA::Vector<QImage>&)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyOriginal(const DAVA::TextureDescriptor *, const DAVA::Vector<QImage>&)), this, SLOT(ReadyOriginal(const DAVA::TextureDescriptor *, const DAVA::Vector<QImage>&)));
    QObject::connect(TextureConvertor::Instance(), SIGNAL(ReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const DAVA::Vector<QImage>&)), this, SLOT(ReadyConverted(const DAVA::TextureDescriptor *, const DAVA::eGPUFamily, const DAVA::Vector<QImage>&)));
}

TextureCache::~TextureCache()
{
    TextureConvertor::Instance()->Release();
}


DAVA::Vector<QImage>  TextureCache::getThumbnail(const DAVA::TextureDescriptor *descriptor)
{
    if(NULL == descriptor)
        return DAVA::Vector<QImage>();
    
    const DAVA::FilePath & path = descriptor->pathname;
	if(cacheThumbnail.find(path) != cacheThumbnail.end())
	{
		// update weight for this cached
		cacheThumbnail[path].weight = curThumbnailWeight++;
        return cacheThumbnail[path].images;
	}
    
	return DAVA::Vector<QImage>();
}


DAVA::Vector<QImage> TextureCache::getOriginal(const DAVA::TextureDescriptor *descriptor)
{
    if(NULL == descriptor)
        return DAVA::Vector<QImage>();

    const DAVA::FilePath & path = descriptor->pathname;
	if(cacheOriginal.find(path) != cacheOriginal.end())
	{
		// update weight for this cached
		cacheOriginal[path].weight = curOriginalWeight++;
		return cacheOriginal[path].images;
	}

	return DAVA::Vector<QImage>();
}

DAVA::Vector<QImage> TextureCache::getConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
    if(NULL == descriptor)
        return DAVA::Vector<QImage>();

    const DAVA::FilePath & path = descriptor->pathname;
	if((gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT) && (cacheConverted[gpu].find(path) != cacheConverted[gpu].end()))
	{
		// update weight for this cached
		cacheConverted[gpu][path].weight = curConvertedWeight[gpu]++;
		return cacheConverted[gpu][path].images;
	}

	return DAVA::Vector<QImage>();
}

void TextureCache::ClearCache()
{
    clearInsteadThumbnails();
    cacheThumbnail.clear();
    
    cacheOriginalSize.clear();
    cacheOriginalFileSize.clear();
    for(DAVA::int32 i = 0; i < DAVA::GPU_FAMILY_COUNT; ++i)
    {
        cacheConvertedSize[i].clear();
        cacheConvertedFileSize[i].clear();
    }
}

void TextureCache::ReadyThumbnail(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage>& image)
{
	setThumbnail(descriptor, image);
    
    setOriginalSize(descriptor);
}


void TextureCache::ReadyOriginal(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage>& image)
{
    setOriginal(descriptor, image);
    setThumbnail(descriptor, image);
    
    setOriginalSize(descriptor);
}

void TextureCache::ReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const DAVA::Vector<QImage>& image)
{
    setConverted(descriptor, gpu, image);
    
    setConvertedSize(descriptor, gpu);
}


void TextureCache::setThumbnail(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage> &images)
{
	if(NULL != descriptor)
	{
        DAVA::Vector<QImage> tmpImages;
        tmpImages.reserve(images.size());
		for(size_t i = 0; i < images.size(); ++i)
		{
            QImage img(THUMBNAIL_SIZE, THUMBNAIL_SIZE, QImage::Format_ARGB32);
            img.fill(QColor(Qt::white));
            
            QPainter painter(&img);
            
            QSize imageSize = images[i].rect().size();
            imageSize.scale(THUMBNAIL_SIZE, THUMBNAIL_SIZE, Qt::KeepAspectRatio);
            int x = (THUMBNAIL_SIZE - imageSize.width()) / 2;
            int y = (THUMBNAIL_SIZE - imageSize.height()) / 2;
            painter.drawImage(QRect(QPoint(x, y), imageSize), images[i]);
            
            painter.end();

            tmpImages.push_back(img);
		}
        
        const DAVA::FilePath & path = descriptor->pathname;

        cacheThumbnail[path] = CacheEntity(tmpImages, curThumbnailWeight++);
        ClearCacheTail(cacheThumbnail, curThumbnailWeight, maxThumbnailCount);

        emit ThumbnailLoaded(descriptor, cacheThumbnail[path].images);
    }
}

void TextureCache::setOriginal(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage> & images)
{
	if(NULL != descriptor)
	{
 		DAVA::Vector<QImage> tmpImages;
        tmpImages.reserve(images.size());

		for(size_t i = 0; i < images.size(); ++i)
		{
			tmpImages.push_back(images[i]);
		}
		
		cacheOriginal[descriptor->pathname] = CacheEntity(tmpImages, curOriginalWeight++);
        ClearCacheTail(cacheOriginal, curOriginalWeight, maxOrigCount);
        
        emit OriginalLoaded(descriptor, cacheOriginal[descriptor->pathname].images);
	}
}

void TextureCache::setConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const DAVA::Vector<QImage>& images)
{
	if( NULL != descriptor && 
		gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT)
	{
		DAVA::Vector<QImage> tmpImages;
        tmpImages.reserve(images.size());
		for(int i = 0; i < (int)images.size(); ++i)
		{
			tmpImages.push_back(images[i]);
		}

		cacheConverted[gpu][descriptor->pathname] = CacheEntity(tmpImages, curConvertedWeight[gpu]++);
        ClearCacheTail(cacheConverted[gpu], curConvertedWeight[gpu], maxConvertedCount);
        
        emit ConvertedLoaded(descriptor, gpu, cacheConverted[gpu][descriptor->pathname].images);
	}
}

void TextureCache::ClearCacheTail(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const size_t currentWeight, const size_t maxWeight)
{
    if(cache.size() > maxWeight)
    {
        size_t weightToRemove = currentWeight - maxWeight;
 
        DAVA::Map<const DAVA::FilePath, CacheEntity>::iterator it = cache.begin();
        while(it != cache.end())
        {
            if(it->second.weight < weightToRemove)
            {
                cache.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }
}


void TextureCache::clearInsteadThumbnails()
{
	cacheOriginal.clear();
	for(int i = DAVA::GPU_UNKNOWN + 1; i < DAVA::GPU_FAMILY_COUNT; ++i)
	{
		cacheConverted[i].clear();
	}
}

void TextureCache::clearThumbnail(const DAVA::TextureDescriptor *descriptor)
{
    RemoveFromCache(cacheThumbnail, descriptor);
    
    RemoveSizeFromCache(cacheOriginalSize, descriptor);
    RemoveSizeFromCache(cacheOriginalFileSize, descriptor);
}

void TextureCache::clearOriginal(const DAVA::TextureDescriptor *descriptor)
{
    RemoveFromCache(cacheOriginal, descriptor);
}

void TextureCache::clearConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
	if(gpu > DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT)
	{
        RemoveFromCache(cacheConverted[gpu], descriptor);
        RemoveSizeFromCache(cacheConvertedSize[gpu], descriptor);
        RemoveSizeFromCache(cacheConvertedFileSize[gpu], descriptor);
	}
}

void TextureCache::RemoveFromCache(DAVA::Map<const DAVA::FilePath, CacheEntity> & cache, const DAVA::TextureDescriptor *descriptor)
{
    if(descriptor)
    {
        DAVA::Map<const DAVA::FilePath, CacheEntity>::iterator found = cache.find(descriptor->pathname);
        if(found != cache.end())
        {
            cache.erase(found);
        }
    }
}

void TextureCache::RemoveSizeFromCache(DAVA::Map<const DAVA::FilePath, DAVA::uint32> & cache, const DAVA::TextureDescriptor *descriptor)
{
    if(descriptor)
    {
        DAVA::Map<const DAVA::FilePath, DAVA::uint32>::iterator found = cache.find(descriptor->pathname);
        if(found != cache.end())
        {
            cache.erase(found);
        }
    }
}


DAVA::uint32 TextureCache::getOriginalSize(const DAVA::TextureDescriptor *descriptor)
{
    return getImageSize(cacheOriginalSize, descriptor);
}

DAVA::uint32 TextureCache::getOriginalFileSize(const DAVA::TextureDescriptor *descriptor)
{
    return getImageSize(cacheOriginalFileSize, descriptor);
}

DAVA::uint32 TextureCache::getConvertedSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
    return getImageSize(cacheConvertedSize[gpu], descriptor);
}

DAVA::uint32 TextureCache::getConvertedFileSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
    return getImageSize(cacheConvertedFileSize[gpu], descriptor);
}

DAVA::uint32 TextureCache::getImageSize(const DAVA::Map<const DAVA::FilePath, DAVA::uint32> & cache, const DAVA::TextureDescriptor *descriptor)
{
    DAVA::Map<const DAVA::FilePath, DAVA::uint32>::const_iterator it = cache.find(descriptor->pathname);
    if(it != cache.end())
    {
        return it->second;
    }
    
    return 0;
}


void TextureCache::setOriginalSize(const DAVA::TextureDescriptor *descriptor)
{
    DAVA::uint32 fileSize = 0;
    if(descriptor->IsCubeMap())
    {
        DAVA::Vector<DAVA::String> faceNames;
        CubemapUtils::GenerateFaceNames(descriptor->pathname.GetAbsolutePathname(), faceNames);
        for(size_t i = 0; i < faceNames.size(); ++i)
        {
            fileSize += QFileInfo(faceNames[i].c_str()).size();
        }
    }
    else
    {
        fileSize = QFileInfo(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str()).size();
    }
    
    cacheOriginalSize[descriptor->pathname] = ImageTools::GetTexturePhysicalSize(descriptor, DAVA::GPU_UNKNOWN);
    cacheOriginalFileSize[descriptor->pathname] = fileSize;
}



void TextureCache::setConvertedSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu)
{
    cacheConvertedSize[gpu][descriptor->pathname] = ImageTools::GetTexturePhysicalSize(descriptor, gpu);

    DAVA::FilePath compressedTexturePath = DAVA::GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, gpu);
    cacheConvertedFileSize[gpu][descriptor->pathname] = QFileInfo(compressedTexturePath.GetAbsolutePathname().c_str()).size();
}

