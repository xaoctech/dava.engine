#ifndef TIMELINE_WIDGET_BASE_H
#define TIMELINE_WIDGET_BASE_H


#include <DAVAEngine.h>
#include <QWidget>
#include <QString>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QScrollBar.h>
#include <qslider.h>

#define SCALE_WIDTH				25

#define GRAPH_HEIGHT			150
#define GRAPH_OFFSET_STEP		10

#define MINIMUM_DISPLAYED_TIME	0.02f
#define ZOOM_STEP				0.1f
#define UI_RECTANGLE_OFFSET		1.5

#define SCROLL_BAR_HEIGHT		12

#define MIN_ZOOM				1.0f
#define MAX_ZOOM				10.0f
#define ZOOM_SLIDER_LENGTH		40

using namespace DAVA;

class TimeLineWidgetBase : public QWidget
{
	Q_OBJECT

public:
	explicit TimeLineWidgetBase(QWidget *parent = 0);
	~TimeLineWidgetBase();
	
	virtual void Init(float32 minT, float32 maxT);
	float32 GetMinBoundary();
	float32 GetMaxBoundary();

signals:
	void ValueChanged();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);

	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent (QKeyEvent *);

	virtual void UpdateSizePolicy()			= 0;
	virtual QRect GetGraphRect() const		= 0;
	virtual QRect GetIncreaseRect() const	= 0;
	virtual QRect GetScaleRect() const		= 0;
	virtual QRect GetDecreaseRect() const	= 0;
	virtual QRect GetSliderRect() const		= 0;

	virtual QRect GetScrollBarRect() const;

	void UpdateScrollBarPosition();
	void UpdateScrollBarSlider();

	void UpdateSliderPosition();
	void UpdateZoomSlider();

	QString float2QString(float32 value) const;
	
	int32 GetIntValue(float32 value) const;

	void PerformZoom(float newScale, bool moveScroll = true);

	void PerformOffset(float value, bool moveScroll = true);

	enum ePositionRelativelyToDrawRect
	{
		POSITION_LEFT,
		POSITION_RIGHT,
		POSITION_INSIDE
	};
	ePositionRelativelyToDrawRect GetPointPositionFromDrawingRect(QPoint point) const; 

protected slots:

	void HandleHorizontalScrollChanged(int value);
	void HandleZoomScrollChanged(int value);

protected:
	QPoint			mouseStartPos;

	float32			minValue;
	float32			maxValue;
	float32			minTime;
	float32			maxTime;
	float32			generalMinTime;
	float32			generalMaxTime;
	float32			minValueLimit;
	float32			maxValueLimit;
	
	enum eGridStyle
	{
		GRID_STYLE_ALL_POSITION,
		GRID_STYLE_LIMITS
	};
	eGridStyle		gridStyle;
	
	QBrush			backgroundBrush;
	bool			isCtrlPressed;
	float32			scale;
	float32			initialTimeInterval;

	QScrollBar	*	horizontalScrollBar;
	QSlider	*		zoomSlider;
};


#endif // TIMELINE_WIDGET_BASE_H
