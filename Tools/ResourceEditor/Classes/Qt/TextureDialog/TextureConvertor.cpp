#include <QtConcurrentRun>
#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include "TextureDialog/TextureConvertor.h"
#include "SceneEditor/PVRConverter.h"

#include "Platform/Qt/QtLayer.h"

TextureConvertor::TextureConvertor()
	: curJobConvert(NULL)
	, curJobOriginal(NULL)
{
	// slot will be called in connector(this) thread
	QObject::connect(&convertWatcher, SIGNAL(finished()), this, SLOT(threadConvertFinished()), Qt::QueuedConnection);
	QObject::connect(&loadOriginalWatcher, SIGNAL(finished()), this, SLOT(threadOriginalFinished()), Qt::QueuedConnection);
}

void TextureConvertor::getPVR( const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, bool forceConver /*= false*/ )
{
	if(NULL != texture && NULL != descriptor)
	{
		JobItem newJob;
		newJob.type = JobItem::JobPVR;
		newJob.texture = texture;
		newJob.descriptor = descriptor;
		newJob.forceConvert = forceConver;

		jobStackConvert.push(newJob);
		jobRunNextConvert();
	}
}

void TextureConvertor::getDXT( const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, bool forceConver /*= false*/ )
{
	if(NULL != texture && NULL != descriptor)
	{
		JobItem newJob;
		newJob.type = JobItem::JobDXT;
		newJob.texture = texture;
		newJob.descriptor = descriptor;
		newJob.forceConvert = forceConver;

		jobStackConvert.push(newJob);
		jobRunNextConvert();
	}
}

void TextureConvertor::loadOriginal(const DAVA::Texture *texture)
{
	if(NULL != texture)
	{
		// we dont care about job-type and descriptor when starting job to load original texture
		JobItem newJob;
		newJob.texture = texture;

		jobStackOriginal.push(newJob);
		jobRunNextOriginal();
	}
}

void TextureConvertor::jobRunNextConvert()
{
	// if there is no already running work
	if((convertWatcher.isFinished() || convertWatcher.isCanceled()) && NULL == curJobConvert)
	{
		// get the new work
		curJobConvert = jobStackConvert.pop();
		if(NULL != curJobConvert)
		{
			curJobConvert->descriptorCopy = *curJobConvert->descriptor;

			switch(curJobConvert->type)
			{
			case JobItem::JobPVR:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadPVR, curJobConvert);
					convertWatcher.setFuture(f);
				}
				break;
			case JobItem::JobDXT:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadDXT, curJobConvert);
					convertWatcher.setFuture(f);
				}
				break;
			default:
				break;
			}
		}
	}

	emit convertStatus(curJobConvert, jobStackConvert.size());
}

void TextureConvertor::jobRunNextOriginal()
{
	// if there is no already running work
	if((loadOriginalWatcher.isFinished() || loadOriginalWatcher.isCanceled())&& NULL == curJobOriginal)
	{
		// get the new work
		curJobOriginal = jobStackOriginal.pop();
		if(NULL != curJobOriginal)
		{
			QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::loadOriginalThread, curJobOriginal);
			loadOriginalWatcher.setFuture(f);
		}
	}
}

QImage TextureConvertor::loadOriginalThread(JobItem *item)
{
	QImage img;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
    
	if(NULL != item && NULL != item->texture)
	{
		DAVA::TextureDescriptor *descriptor = item->texture->CreateDescriptor();
		img = QImage(descriptor->GetSourceTexturePathname().c_str());
		SafeRelease(descriptor);
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return img;
}

QImage TextureConvertor::convertThreadPVR(JobItem *item)
{
	QImage qtImage;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

	if(NULL != item && item->descriptorCopy.pvrCompression.format != DAVA::FORMAT_INVALID)
	{
		DAVA::String sourcePath = item->descriptorCopy.GetSourceTexturePathname();
		DAVA::String outputPath = PVRConverter::Instance()->GetPVRToolOutput(sourcePath);

		if(!outputPath.empty())
		{
			if(item->forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath))
			{
				QProcess p;
				DAVA::String command = PVRConverter::Instance()->GetCommandLinePVR(sourcePath, item->descriptorCopy);
				DAVA::Logger::Info("%s$: %s", DAVA::FileSystem::Instance()->GetCurrentWorkingDirectory().c_str(), command.c_str());

				p.start(QString(command.c_str()));
				p.waitForFinished(-1);

				if(QProcess::NormalExit != p.exitStatus())
				{
					DAVA::Logger::Error("Convertor process crushed\n");
				}

				if(0 != p.exitCode())
				{
					DAVA::Logger::Error("Convertor exit with error %d\n", p.exitCode());
					DAVA::Logger::Error("Stderror: %s", p.readAllStandardError().constData());
				}
			}

			item->descriptorCopy.UpdateDateAndCrcForFormat(DAVA::PVR_FILE);
			item->descriptorCopy.Save();

			std::vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

			if(davaImages.size() > 0)
			{
				DAVA::Image *davaImage = davaImages[0];
				qtImage = fromDavaImage(davaImage);
			}

			for_each(davaImages.begin(), davaImages.end(),  DAVA::SafeRelease< DAVA::Image>);
		}
	}
	else
	{
		QRect r(0, 0, item->texture->width, item->texture->height);
		qtImage = QImage(r.size(), QImage::Format_ARGB32);

		QPainter p(&qtImage);
        p.setBrush(QBrush(QColor(100, 100, 100)));
        p.setPen(QColor(255, 255, 255));
        p.drawRect(r);
		p.drawText(r, "No image", QTextOption(Qt::AlignCenter));
	}

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return qtImage;
}

QImage TextureConvertor::convertThreadDXT(JobItem *item)
{
	QImage convertedImage;

    void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();

    
	// TODO:
	// convert
	// ...

	// debug -->
	convertedImage = QImage(item->texture->width, item->texture->height, QImage::Format_ARGB32);
	QPainter p(&convertedImage);
	p.setBrush(QColor(0,255,0));
	p.drawEllipse(QPoint(item->texture->width/2, item->texture->height/2), item->texture->width/2 - 4, item->texture->height/2 - 4);
	// <--

    DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
	return convertedImage;
}

void TextureConvertor::threadOriginalFinished()
{
	if(loadOriginalWatcher.isFinished() && NULL != curJobOriginal)
	{
		emit readyOriginal(curJobOriginal->texture, loadOriginalWatcher.result());

		delete curJobOriginal;
		curJobOriginal = NULL;
	}

	jobRunNextOriginal();
}

void TextureConvertor::threadConvertFinished()
{
	if(convertWatcher.isFinished() && NULL != curJobConvert)
	{
		switch(curJobConvert->type)
		{
		case JobItem::JobPVR:
			emit readyPVR(curJobConvert->texture, &curJobConvert->descriptorCopy, convertWatcher.result());
			break;
		case JobItem::JobDXT:
			emit readyDXT(curJobConvert->texture, &curJobConvert->descriptorCopy, convertWatcher.result());
			break;
		default:
			break;
		}

		delete curJobConvert;
		curJobConvert = NULL;
	}

	jobRunNextConvert();
}

QImage TextureConvertor::fromDavaImage(DAVA::Image *image)
{
	QImage qtImage;

	if(NULL != image)
	{
		QRgb *line;

		switch(image->format)
		{
		case DAVA::FORMAT_PVR4:
		case DAVA::FORMAT_PVR2:
		case DAVA::FORMAT_RGBA8888:
			{
				DAVA::uint32 *data = (DAVA::uint32 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA8888 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = c & 0xFF00FF00 | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA5551:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA5551 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 11) & 0x1f;
						g = (c >> 6) & 0x1f;
						b = (c >> 1) & 0x1f;
						a = (c & 0x1) ? 0xff000000 : 0x0;

						line[x] = (a | (r << (16 + 3)) | (g << (8 + 3)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGBA4444:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA4444 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						r = (c >> 12) & 0xf;
						g = (c >> 8) & 0xf;
						b = (c >> 4) & 0xf;
						a = (c & 0xf);

						line[x] = ((a << (24 + 4)) | (r << (16 + 4)) | (g << (8+4)) | (b << 4));
					}
				}
			}
			break;

		case DAVA::FORMAT_RGB565:
			{
				DAVA::uint16 *data = (DAVA::uint16 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						DAVA::uint32 a;
						DAVA::uint32 r;
						DAVA::uint32 g;
						DAVA::uint32 b;

						c = data[y * image->width + x];
						a = 0xff;
						r = (c >> 11) & 0x1f;
						g = (c >> 5) & 0x3f;
						b = c & 0x1f;

						line[x] = ((a << 24) | (r << (16 + 3)) | (g << (8 + 2)) | (b << 3));
					}
				}
			}
			break;

		case DAVA::FORMAT_A8:
			{
				DAVA::uint8 *data = (DAVA::uint8 *) image->data;
				DAVA::uint32 c;

				qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA565 into Qt ARGB8888
				for (int y = 0; y < image->height; y++) 
				{
					line = (QRgb *) qtImage.scanLine(y);
					for (int x = 0; x < image->width; x++) 
					{
						c = data[y * image->width + x];
						line[x] = ((0xff << 24) | (c << 16) | (c << 8) | c);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return qtImage;
}
