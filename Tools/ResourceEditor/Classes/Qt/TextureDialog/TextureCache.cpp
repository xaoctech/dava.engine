#include "TextureDialog/TextureCache.h"


QImage TextureCache::getOriginal(const DAVA::Texture *texture)
{
	QImage img;

	if(NULL != texture && cacheOriginal.contains(texture))
	{
		img = cacheOriginal[texture];
	}

	return img;
}

QImage TextureCache::getPVR(const DAVA::Texture *texture)
{
	QImage img;

	if(NULL != texture && cacheOriginal.contains(texture))
	{
		img = cachePVR[texture];
	}

	return img;
}

QImage TextureCache::getDXT(const DAVA::Texture *texture)
{
	QImage img;

	if(NULL != texture && cacheOriginal.contains(texture))
	{
		img = cacheDXT[texture];
	}

	return img;
}

void TextureCache::setOriginal(const DAVA::Texture *texture, const QImage &image)
{
	if(NULL != texture)
	{
		cacheOriginal[texture] = image;
	}
}

void TextureCache::setPVR(const DAVA::Texture *texture, const QImage &image)
{
	if(NULL != texture)
	{
		cachePVR[texture] = image;
	}
}

void TextureCache::setDXT(const DAVA::Texture *texture, const QImage &image)
{
	if(NULL != texture)
	{
		cacheDXT[texture] = image;
	}
}

void TextureCache::clearAll()
{
	cacheOriginal.clear();
	cachePVR.clear();
	cacheDXT.clear();
}

void TextureCache::clearOriginal(const DAVA::Texture *texture)
{
	cacheOriginal.remove(texture);
}

void TextureCache::clearPVR(const DAVA::Texture *texture)
{
	cachePVR.remove(texture);
}

void TextureCache::clearDXT(const DAVA::Texture *texture)
{
	cacheDXT.remove(texture);
}
