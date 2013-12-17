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



#ifndef __TEXTURE_CACHE_H__
#define __TEXTURE_CACHE_H__

#include "Base/Singleton.h"
#include "Render/TextureDescriptor.h"

#include <QMap>
#include <QImage>

class TextureCache : public QObject, public DAVA::Singleton<TextureCache>
{
	Q_OBJECT

private:
	struct CacheEntity
	{
		CacheEntity()
        : weight(0)
		{ }
        
		CacheEntity(const DAVA::Vector<QImage> & _images, const size_t _weight)
        : images(_images), weight(_weight)
		{ }
        
		DAVA::Vector<QImage> images;
		size_t weight;
	};
    
public:
    
    TextureCache();
    ~TextureCache();

    static const int THUMBNAIL_SIZE = 64;
    
    DAVA::Vector<QImage> getThumbnail(const DAVA::TextureDescriptor *descriptor);
	DAVA::Vector<QImage> getOriginal(const DAVA::TextureDescriptor *descriptor);
	DAVA::Vector<QImage> getConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);

	void clearInsteadThumbnails();
	void clearConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu);
    
signals:

    void ThumbnailLoaded(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage> & image);
    void OriginalLoaded(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage> & image);
    void ConvertedLoaded(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const DAVA::Vector<QImage> & image);
    
protected slots:
    
    void ReadyOriginal(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage>& image);
	void ReadyConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const DAVA::Vector<QImage>& image);

protected:
    
    void setThumbnail(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage>& images);
	void setOriginal(const DAVA::TextureDescriptor *descriptor, const DAVA::Vector<QImage>& images);
	void setConverted(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily gpu, const DAVA::Vector<QImage>& images);

    void clearThumbnail(const DAVA::TextureDescriptor *descriptor);
	void clearOriginal(const DAVA::TextureDescriptor *descriptor);

    
    void ClearCacheTail(QMap<const DAVA::TextureDescriptor*, CacheEntity> & cache, const size_t currentWeight, const size_t maxWeight);
    
private:

	size_t curThumbnailWeight;
	size_t curOriginalWeight;
	size_t curConvertedWeight[DAVA::GPU_FAMILY_COUNT];

    static const size_t maxThumbnailCount = 200;
	static const size_t maxOrigCount = 20;
	static const size_t maxConvertedCount = 7; // per gpu

	QMap<const DAVA::TextureDescriptor*, CacheEntity> cacheThumbnail;
	QMap<const DAVA::TextureDescriptor*, CacheEntity> cacheOriginal;
	QMap<const DAVA::TextureDescriptor*, CacheEntity> cacheConverted[DAVA::GPU_FAMILY_COUNT];
};

#endif // __TEXTURE_CACHE_H__
