#ifndef __TEXTURE_CONVERTOR_H__
#define __TEXTURE_CONVERTOR_H__

#include <QObject>
#include <QImage>
#include <QFutureWatcher>
#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"
#include "Render/RenderManager.h"
#include "TextureDialog/TextureConvertorWork.h"

class TextureConvertor : public QObject, public DAVA::Singleton<TextureConvertor>
{
	Q_OBJECT

public:
	TextureConvertor();

	void loadOriginal(const DAVA::Texture *texture);
	void getPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor);
	void getDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor);

private:
	QFutureWatcher<QImage> loadOriginalWatcher;
	QFutureWatcher<QImage> convertWatcher;

	WorkStack workStack;
	WorkItem *curWork;

	const DAVA::Texture* curOriginalTexture;

	void workRunNext();

	QImage loadOriginalThread(const DAVA::Texture *texture);
	QImage convertThreadPVR(const WorkItem *item);
	QImage convertThreadDXT(const WorkItem *item);

signals:
	void readyOriginal(const DAVA::Texture *texture, const QImage &image);
	void readyPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor, const QImage &image);
	void readyDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor, const QImage &image);
	void convertStatus(const WorkItem *workCur, int workLeft);

private slots:
	void threadOriginalFinished();
	void threadConvertFinished();

};

#endif // __TEXTURE_CONVERTOR_H__
