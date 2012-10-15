#ifndef __TEXTURE_SCROLL_AREA_H__
#define __TEXTURE_SCROLL_AREA_H__

#include <QGraphicsView>

class QImage;

class TextureScrollArea : public QGraphicsView
{
	Q_OBJECT

public:
	enum TextureColorChannels
	{
		ChannelNo = 0,

		ChannelR = 0x1,
		ChannelG = 0x2,
		ChannelB = 0x4,
		ChannelA = 0x8,

		ChannelAll = 0xFFFFFFFF
	};

	TextureScrollArea(QWidget* parent=0);
	~TextureScrollArea();

	void setImage(const QImage &image);
	void setColorChannel(int mask);

	float getZoom();
	int   getZoomPercent();
	void  setZoomPercent();
	void  fitZoom();

public slots:	
	void texturePos(const QPoint &pos);
	void textureZoom(const float &zoom);

signals:
	void texturePosChanged(QPoint pos);
	void textureZoomChanged(const float &zoom);

protected:
	void setScene(QGraphicsScene *scene);
	virtual void scrollContentsBy(int dx, int dy);
	virtual void wheelEvent(QWheelEvent * e);

	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

private:
	int textureColorMask;

	bool mouseInMoveState;
	QPoint mousePressPos;
	QPoint mousePressScrollPos;

	QImage currentTextureImage;

	QGraphicsScene *textureScene;
	QGraphicsPixmapItem *texturePixmap;
	float zoomFactor;

	void applyCurrentImageToScenePixmap();
};

#endif // __TEXTURE_SCROLL_AREA_H__
