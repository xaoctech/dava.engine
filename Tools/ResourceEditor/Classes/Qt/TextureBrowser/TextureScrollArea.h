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
	QImage getImage();
	void setColorChannel(int mask);

	QColor getPixelColor(QPoint pos);
	float getTextureZoom();

	void borderShow(bool show);
	void bgmaskShow(bool show);
	void waitbarShow(bool show);

	void resetTexturePosZoom();

public slots:	
	void setTexturePos(const QPoint &pos);
	void setTextureZoom(const float &zoom);

signals:
	void texturePosChanged(const QPoint &pos);
	void textureZoomChanged(const float &zoom);

	void mouseOverPixel(const QPoint &pos);
	void mouseWheel(int delta);

protected:
	virtual void drawBackground(QPainter * painter, const QRectF & rect);
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

	QGraphicsRectItem *textureBorder;
	QGraphicsProxyWidget *waitBar;

	QImage currentTextureImage;

	QGraphicsScene *textureScene;
	QGraphicsPixmapItem *texturePixmap;
	float zoomFactor;

	bool tiledBgDoDraw;
	QPixmap tiledBgPixmap;

	void sutupCustomTiledBg();

	void applyCurrentImageToScenePixmap();
	void applyCurrentImageBorder();
	void adjustWaitBarPos();
};

#endif // __TEXTURE_SCROLL_AREA_H__
