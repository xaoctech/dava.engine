#include <QtConcurrentRun>
#include <QPainter>
#include <QProcess>
#include <QTextOption>
#include "TextureDialog/TextureConvertor.h"
#include "SceneEditor/PVRConverter.h"

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
	if(convertWatcher.isFinished() && NULL == curJobConvert)
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
	if(loadOriginalWatcher.isFinished() && NULL == curJobOriginal)
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

	if(NULL != item && NULL != item->texture)
	{
		DAVA::TextureDescriptor *descriptor = item->texture->CreateDescriptor();
		img = QImage(descriptor->GetSourceTexturePathname().c_str());
		delete descriptor;
	}

	return img;
}

QImage TextureConvertor::convertThreadPVR(JobItem *item)
{
	QImage qtImage;

	if(NULL != item && item->descriptorCopy.pvrCompression.format != DAVA::FORMAT_INVALID)
	{
		DAVA::String sourcePath = item->descriptorCopy.GetSourceTexturePathname();
		DAVA::String outputPath = PVRConverter::Instance()->GetPVRToolOutput(sourcePath);

		if(!outputPath.empty())
		{
			// forced to convert or output file not exists
			// TODO: add check for descriptor CRC
			// ...
			if(item->forceConvert || !DAVA::FileSystem::Instance()->IsFile(outputPath) /* || bad_crc */)
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

			// TODO:
			// item->descriptor.UpdateDateAndCrcForFormat(DAVA::PVR_FILE);
			// item->descriptor.Save();

			std::vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

			if(davaImages.size() > 0)
			{
				DAVA::Image *davaImage = davaImages[0];
				qtImage = QImage(davaImage->width, davaImage->height, QImage::Format_ARGB32);

				// convert DAVA:RGBA8888 into Qt ARGB8888
				{
					QRgb *data = (QRgb *) davaImage->data;
					QRgb *line;
					QRgb c;

					for (int y = 0; y < davaImage->height; y++) 
					{
						line = (QRgb *) qtImage.scanLine(y);
						for (int x = 0; x < davaImage->width; x++) 
						{
							c = data[y * davaImage->width + x];
							line[x] = c & 0xFF00FF00 | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
						}
					}
				}
			}

            DAVA::SafeRelease(davaImages.begin(), davaImages.end());
		}
	}
	else
	{
		QRect r(0, 0, item->texture->width, item->texture->height);
		qtImage = QImage(r.size(), QImage::Format_ARGB32);

		QPainter p(&qtImage);
		p.drawText(r, "No image", QTextOption(Qt::AlignCenter));
	}

	return qtImage;
}

QImage TextureConvertor::convertThreadDXT(JobItem *item)
{
	QImage convertedImage;

	// TODO:
	// convert
	// ...

	// debug -->
	convertedImage = QImage(item->texture->width, item->texture->height, QImage::Format_ARGB32);
	QPainter p(&convertedImage);
	p.setBrush(QColor(0,255,0));
	p.drawEllipse(QPoint(item->texture->width/2, item->texture->height/2), item->texture->width/2 - 4, item->texture->height/2 - 4);
	// <--

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
