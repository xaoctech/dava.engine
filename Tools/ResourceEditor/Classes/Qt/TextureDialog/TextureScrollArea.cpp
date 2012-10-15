#include "TextureScrollArea.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QApplication>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>
#include <QImage>
#include <QPixmap>

TextureScrollArea::TextureScrollArea(QWidget* parent/* =0 */)
	: QGraphicsView(parent)
	, textureColorMask((int) ChannelAll)
	, mouseInMoveState(false)
	, textureScene(NULL)
	, zoomFactor(1.0)
{
	textureScene = new QGraphicsScene();

	setRenderHints((QPainter::RenderHints) 0);
	setScene(textureScene);

	textureScene->setBackgroundBrush(QBrush(QColor(0,0,0)));
	texturePixmap = textureScene->addPixmap(QPixmap());

	//translate(0.5, 0.5);
}

TextureScrollArea::~TextureScrollArea()
{ }

void TextureScrollArea::setImage(const QImage &image)
{
	textureZoom(1.0);
	texturePos(QPoint(0,0));

	currentTextureImage = image;

	applyCurrentImageToScenePixmap();
}

void TextureScrollArea::setColorChannel(int mask)
{
	textureColorMask = mask;
	applyCurrentImageToScenePixmap();
}

void TextureScrollArea::setScene(QGraphicsScene *scene)
{
	QGraphicsView::setScene(scene);
}

float TextureScrollArea::getZoom()
{
	return zoomFactor;
}

int TextureScrollArea::getZoomPercent()
{
	return 0;
}

void TextureScrollArea::setZoomPercent()
{

}

void TextureScrollArea::fitZoom()
{

}

void TextureScrollArea::textureZoom(const float &zoom)
{
	if(zoom != zoomFactor)
	{
		zoomFactor = zoom;


		resetTransform();
		scale(zoomFactor, zoomFactor);
		emit textureZoomChanged(zoomFactor);
	}
}

void TextureScrollArea::texturePos(const QPoint &pos)
{
	horizontalScrollBar()->setValue(pos.x());
	verticalScrollBar()->setValue(pos.y());
}

void TextureScrollArea::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx, dy);
	emit texturePosChanged(QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value()));
}

void TextureScrollArea::wheelEvent(QWheelEvent * e)
{
	if(e->delta() > 0)
	{
		textureZoom(getZoom() * 1.1);
	}
	else
	{
		textureZoom(getZoom() / 1.1);
	}
}

void TextureScrollArea::mouseMoveEvent(QMouseEvent *event)
{
	QGraphicsView::mouseMoveEvent(event);

	if(mouseInMoveState)
	{
		int mouseDx = event->pos().x() - mousePressPos.x();
		int mouseDy = event->pos().y() - mousePressPos.y();

		horizontalScrollBar()->setValue(mousePressScrollPos.x() - mouseDx);
		verticalScrollBar()->setValue(mousePressScrollPos.y() - mouseDy);
	}
}

void TextureScrollArea::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);

	mouseInMoveState = true;
	mousePressPos = event->pos();
	mousePressScrollPos.setX(horizontalScrollBar()->value());
	mousePressScrollPos.setY(verticalScrollBar()->value());
	viewport()->setProperty("cursor", QVariant(QCursor(Qt::ClosedHandCursor)));
}

void TextureScrollArea::mouseReleaseEvent(QMouseEvent *event)
{
	QGraphicsView::mouseReleaseEvent(event);

	mouseInMoveState = false;
	viewport()->setProperty("cursor", QVariant(QCursor(Qt::OpenHandCursor)));
}

void TextureScrollArea::applyCurrentImageToScenePixmap()
{
	// TODO: optimize this code

	if(~textureColorMask)
	{
		int mask = 0xFFFFFFFF;
		int maskA = 0;

		QImage tmpImage = currentTextureImage.convertToFormat(QImage::Format_ARGB32);

		if(!(textureColorMask & ChannelR)) mask &= 0xFF00FFFF;
		if(!(textureColorMask & ChannelG)) mask &= 0xFFFF00FF;
		if(!(textureColorMask & ChannelB)) mask &= 0xFFFFFF00;
		if(!(textureColorMask & ChannelA)) maskA |= 0xFF000000;

		if(mask == 0xFF000000)
		{
			maskA ^= 0xFF000000;

			// only alpha, so show it
			for (int y = 0; y < tmpImage.height(); y++) 
			{
				QRgb *line = (QRgb *) tmpImage.scanLine(y);
				for (int x = 0; x < tmpImage.width(); x++) 
				{
					int c = (line[x] & 0xFF000000) >> 24;
					line[x] = (maskA | c << 16 | c << 8 | c);
				}
			}
		}
		else
		{
			for (int y = 0; y < tmpImage.height(); y++) 
			{
				QRgb *line = (QRgb *) tmpImage.scanLine(y);
				for (int x = 0; x < tmpImage.width(); x++) 
				{
					line[x] &= mask;
					line[x] |= maskA;
				}
			}
		}

		QPixmap pixmap = QPixmap::fromImage(tmpImage);
		textureScene->setSceneRect(pixmap.rect());
		texturePixmap->setPixmap(pixmap);
	}
	else
	{
		QPixmap pixmap = QPixmap::fromImage(currentTextureImage);
		textureScene->setSceneRect(pixmap.rect());
		texturePixmap->setPixmap(pixmap);
	}
}