#include <QtConcurrentRun>
#include <QPainter>
#include "TextureDialog/TextureConvertor.h"
#include "SceneEditor/PVRConverter.h"

TextureConvertor::TextureConvertor()
	: curWork(NULL)
	, curOriginalTexture(NULL)
{
	// slot will be called in connector(this) thread
	QObject::connect(&convertWatcher, SIGNAL(finished()), this, SLOT(threadConvertFinished()), Qt::QueuedConnection);
	QObject::connect(&loadOriginalWatcher, SIGNAL(finished()), this, SLOT(threadOriginalFinished()), Qt::QueuedConnection);
}

void TextureConvertor::getPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor)
{
	workStack.push(WorkItem::WorkPVR, texture, descriptor);
	workRunNext();
}

void TextureConvertor::getDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor &descriptor)
{
	workStack.push(WorkItem::WorkDXT, texture, descriptor);
	workRunNext();
}

void TextureConvertor::loadOriginal(const DAVA::Texture *texture)
{
	loadOriginalWatcher.cancel();

	curOriginalTexture = texture;
	QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::loadOriginalThread, texture);
	loadOriginalWatcher.setFuture(f);
}

void TextureConvertor::workRunNext()
{
	// if there is no already running work
	if(convertWatcher.isFinished() && NULL == curWork)
	{
		// get the new work
		curWork = workStack.pop();
		if(NULL != curWork)
		{
			switch(curWork->type)
			{
			case WorkItem::WorkPVR:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadPVR, curWork);
					convertWatcher.setFuture(f);
				}
				break;
			case WorkItem::WorkDXT:
				{
					QFuture<QImage> f = QtConcurrent::run(this, &TextureConvertor::convertThreadDXT, curWork);
					convertWatcher.setFuture(f);
				}
				break;
			default:
				break;
			}
		}
	}

	emit convertStatus(curWork, workStack.size());
}

QImage TextureConvertor::loadOriginalThread(const DAVA::Texture *texture)
{
	QImage img;

	if(NULL != texture)
	{
		DAVA::TextureDescriptor *descriptor = DAVA::Texture::CreateDescriptorForTexture(texture->GetPathname());
		img = QImage(descriptor->GetSourceTexturePathname().c_str());
		delete descriptor;
	}

	return img;
}

QImage TextureConvertor::convertThreadPVR(const WorkItem *item)
{
	QImage qtConvertedImage;

	if(NULL != item)
	{
		DAVA::String sourcePath = item->descriptor.GetSourceTexturePathname();
		DAVA::String outputPath = PVRConverter::Instance()->ConvertPngToPvr(sourcePath, item->descriptor);

		if(!outputPath.empty())
		{
			item->descriptor.UpdateDateAndCrc();
			item->descriptor.Save();

			std::vector<DAVA::Image *> davaImages = DAVA::ImageLoader::CreateFromFile(outputPath);

			if(davaImages.size() > 0)
			{
				DAVA::Image *davaImage = davaImages[0];
				QImage::Format qtImgFormat = QImage::Format_Invalid;

				switch(davaImage->GetPixelFormat())
				{
				case DAVA::FORMAT_RGBA8888:
					qtImgFormat = QImage::Format_ARGB32;
					break;
				}

				if(QImage::Format_Invalid != qtImgFormat)
				{
					qtConvertedImage = QImage(davaImage->width, davaImage->height, QImage::Format_ARGB32);

					QRgb *data = (QRgb *) davaImage->data;
					QRgb *line;
					QRgb c;

					for (int y = 0; y < davaImage->height; y++) 
					{
						line = (QRgb *) qtConvertedImage.scanLine(y);
						for (int x = 0; x < davaImage->width; x++) 
						{
							c = data[y * davaImage->width + x];
							line[x] = c & 0xFF00FF00 | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
						}
					}
				}
			}

			for(unsigned int i = 0; i < davaImages.size(); ++i)
			{
				DAVA::SafeRelease(davaImages[i]);
			}
		}
	}

	/*
	// debug -->
	convertedImage = QImage(item->texture->width, item->texture->height, QImage::Format_ARGB32);
	QPainter p(&convertedImage);
	p.setBrush(QColor(255,0,0));
	p.drawEllipse(QPoint(item->texture->width/2, item->texture->height/2), item->texture->width/2 - 4, item->texture->height/2 - 4);
	Sleep(1000);
	// <--
	*/

	return qtConvertedImage;
}

QImage TextureConvertor::convertThreadDXT(const WorkItem *item)
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
	Sleep(1000);
	// <--

	return convertedImage;
}

void TextureConvertor::threadOriginalFinished()
{
	emit readyOriginal(curOriginalTexture, loadOriginalWatcher.result());
	curOriginalTexture = NULL;
}

void TextureConvertor::threadConvertFinished()
{
	if(convertWatcher.isFinished() && NULL != curWork)
	{
		switch(curWork->type)
		{
		case WorkItem::WorkPVR:
			emit readyPVR(curWork->texture, curWork->descriptor, convertWatcher.result());
			break;
		case WorkItem::WorkDXT:
			emit readyDXT(curWork->texture, curWork->descriptor, convertWatcher.result());
			break;
		default:
			break;
		}

		delete curWork;
		curWork = NULL;
	}

	workRunNext();
}
