#ifndef __TEXTURE_CONVERTOR_H__
#define __TEXTURE_CONVERTOR_H__

#include <QObject>
#include <QImage>
#include <QFutureWatcher>
#include "DAVAEngine.h"
#include "Render/TextureDescriptor.h"
#include "Render/RenderManager.h"
#include "TextureDialog/TextureConvertorWork.h"

#define CONVERT_JOB_COUNT 2

class TextureConvertor : public QObject, public DAVA::Singleton<TextureConvertor>
{
	Q_OBJECT

public:
	TextureConvertor();

	void loadOriginal(const DAVA::Texture *texture);
	void getPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, bool forceConver = false);
	void getDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, bool forceConver = false);

private:
	QFutureWatcher<QImage> loadOriginalWatcher;
	QFutureWatcher<QImage> convertWatcher[CONVERT_JOB_COUNT];

	JobStack jobStackConvert;
	JobItem *curJobConvert[CONVERT_JOB_COUNT];

	JobStack jobStackOriginal;
	JobItem *curJobOriginal;

	void jobRunNextConvert();
	void jobRunNextOriginal();

	QImage loadOriginalThread(JobItem *item);
	QImage convertThreadPVR(JobItem *item);
	QImage convertThreadDXT(JobItem *item);

	QImage fromDavaImage(DAVA::Image *image);

	int jobGetConvertFreeIndex();
	int jobGetConvertIndex(QFutureWatcher<QImage> *watcher);

signals:
	void readyOriginal(const DAVA::Texture *texture, const QImage &image);
	void readyPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void readyDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, const QImage &image);
	void convertStatus(const JobItem *jobCur, int jobLeft);

private slots:
	void threadOriginalFinished();
	void threadConvertFinished();

};

#endif // __TEXTURE_CONVERTOR_H__
