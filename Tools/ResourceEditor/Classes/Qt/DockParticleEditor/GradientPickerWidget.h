#ifndef __RESOURCE_EDITOR_GRADIENTPICKERWIDGET_H__
#define __RESOURCE_EDITOR_GRADIENTPICKERWIDGET_H__

#include <QWidget>

#include <DAVAEngine.h>

using namespace DAVA;

class GradientPickerWidget: public QWidget
{
	Q_OBJECT
public:
	explicit GradientPickerWidget(QWidget *parent = 0);
	~GradientPickerWidget();
	
	void Init(float32 minT, float32 maxT, const QString& legend = "");
	
	void SetLimits(float32 minT, float32 maxT);
	void SetValues(const Vector<PropValue<Color> >& values);
	bool GetValues(Vector<PropValue<Color> >* values);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void leaveEvent(QEvent *);

signals:
	void ValueChanged();

public slots:

private:
	float32 minTime;
	float32 maxTime;
	Vector<std::pair<float32, Color> > points;
	int32 selectedPointIndex;
	bool showCursorPos;
	QPoint cursorPos;
	QString legend;

	QBrush backgroundBrush;
	QPixmap tiledPixmap;

	QRect GetGraphRect();
	QRect GetTextRect();
	
	static bool ComparePoints(const std::pair<float32, Color>& a, const std::pair<float32, Color>& b);
	static bool CompareIndices(int32 a, int32 b);

	bool AddPoint(float32 point);
	bool AddColorPoint(float32 point, const Color& color);
	bool SetCurrentPointColor(const Color& color);
	bool SetPointColor(uint32 index, const Color& color);
	Color GetCurrentPointColor();
	Color GetPointColor(uint32 index);
	bool DeleteCurrentPoint();
	bool DeletePoint(uint32 index);
	bool DeletePoints(Vector<int32> indices);
	void ClearPoints();
	
	float32 GetTimeFromCursorPos(float32 xPos);
	float32 GetCursorPosFromTime(float32 time);
	float32 GetGradientPosFromTime(float32 time);

	Vector<QRectF> GetMarkerRects();
	Vector<int32> GetMarkersFromCursorPos(const QPoint& point);
};

#endif // __RESOURCE_EDITOR_GRADIENTPICKERWIDGET_H__
