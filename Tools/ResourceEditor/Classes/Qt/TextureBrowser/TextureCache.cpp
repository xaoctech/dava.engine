/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "TextureBrowser/TextureCache.h"


QImage TextureCache::getOriginal(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cacheOriginal[descriptor];
	}

	return img;
}

QImage TextureCache::getPVR(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cachePVR[descriptor];
	}

	return img;
}

QImage TextureCache::getDXT(const DAVA::TextureDescriptor *descriptor)
{
	QImage img;

	if(NULL != descriptor && cacheOriginal.contains(descriptor))
	{
		img = cacheDXT[descriptor];
	}

	return img;
}

void TextureCache::setOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cacheOriginal[descriptor] = image;
	}
}

void TextureCache::setPVR(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cachePVR[descriptor] = image;
	}
}

void TextureCache::setDXT(const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	if(NULL != descriptor)
	{
		cacheDXT[descriptor] = image;
	}
}

void TextureCache::clearAll()
{
	cacheOriginal.clear();
	cachePVR.clear();
	cacheDXT.clear();
}

void TextureCache::clearOriginal(const DAVA::TextureDescriptor *descriptor)
{
	cacheOriginal.remove(descriptor);
}

void TextureCache::clearPVR(const DAVA::TextureDescriptor *descriptor)
{
	cachePVR.remove(descriptor);
}

void TextureCache::clearDXT(const DAVA::TextureDescriptor *descriptor)
{
	cacheDXT.remove(descriptor);
}
