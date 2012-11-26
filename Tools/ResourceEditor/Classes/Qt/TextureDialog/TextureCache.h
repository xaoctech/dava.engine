#ifndef __TEXTURE_CACHE_H__
#define __TEXTURE_CACHE_H__

#include "DAVAEngine.h"
#include <QMap>
#include <QImage>

#include "TextureDialog/TextureConvertor.h"

class TextureCache : public QObject, public DAVA::Singleton<TextureCache>
{
	Q_OBJECT

public:
	QImage getOriginal(const DAVA::Texture *texture);
	QImage getPVR(const DAVA::Texture *texture);
	QImage getDXT(const DAVA::Texture *texture);

	void setOriginal(const DAVA::Texture *texture, const QImage &image);
	void setPVR(const DAVA::Texture *texture, const QImage &image);
	void setDXT(const DAVA::Texture *texture, const QImage &image);

	void clearAll();
	void clearOriginal(const DAVA::Texture *texture);
	void clearPVR(const DAVA::Texture *texture);
	void clearDXT(const DAVA::Texture *texture);

private:
	QMap<const DAVA::Texture*, QImage> cacheOriginal;
	QMap<const DAVA::Texture*, QImage> cachePVR;
	QMap<const DAVA::Texture*, QImage> cacheDXT;
};

#endif // __TEXTURE_CACHE_H__
