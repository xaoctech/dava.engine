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
	QPixmap pixmap = QPixmap::fromImage(currentTextureImage);
	textureScene->setSceneRect(pixmap.rect());
	texturePixmap->setPixmap(pixmap);
}