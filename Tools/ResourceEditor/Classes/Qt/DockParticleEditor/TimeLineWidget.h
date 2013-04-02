#ifndef TIMELINE_H
#define TIMELINE_H


#include <DAVAEngine.h>
#include <QWidget>
#include <QString>
#include <QDialog>
#include <QDoubleSpinBox>

using namespace DAVA;

class TimeLineWidget : public QWidget
{
	Q_OBJECT

public:
	explicit TimeLineWidget(QWidget *parent = 0);
	~TimeLineWidget();
	
	void Init(float32 minT, float32 maxT, bool updateSizeState, bool aliasLinePoint = false, bool allowDeleteLine = true, bool integer = false);
	void Init(float32 minT, float32 maxT, float32 generalMinT, float32 generalMaxT, bool updateSizeState, bool aliasLinePoint = false, bool allowDeleteLine = true, bool integer = false);
	void SetMinLimits(float32 minV);
	void SetMaxLimits(float32 maxV);
	float32 GetMinBoundary();
	float32 GetMaxBoundary();
	void EnableLock(bool enable);
	void SetVisualState(KeyedArchive* visualStateProps);
	void GetVisualState(KeyedArchive* visualStateProps);
	
	void AddLine(uint32 lineId, const Vector< PropValue<float32> >& line, const QColor& color, const QString& legend = "");
	void AddLines(const Vector< PropValue<Vector2> >& lines, const Vector<QColor>& colors, const Vector<QString>& legends);
	void AddLines(const Vector< PropValue<Vector3> >& lines, const Vector<QColor>& colors, const Vector<QString>& legends);
	
	bool GetValue(uint32 lineId, Vector< PropValue<float32> >* line) const;
	bool GetValues(Vector< PropValue<Vector2> >* lines);
	bool GetValues(Vector< PropValue<Vector3> >* lines);
	
	static bool SortPoints(const Vector2& i, const Vector2& j);
	
signals:
	void TimeLineUpdated();
	void ValueChanged();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void leaveEvent(QEvent *);
	virtual void wheelEvent(QWheelEvent*);
    virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent (QKeyEvent *);

private:
	typedef Vector<Vector2> LOGIC_POINTS;
	
	QRect GetGraphRect() const;
	void DrawLine(QPainter* painter, uint32 lineId);
	QPoint GetDrawPoint(const Vector2& point) const;
	Vector2 GetLogicPoint(const QPoint& point) const;
	QRect GetPointRect(const QPoint& point) const;
	
	QRect GetLineEnableRect(uint32 lineId) const;
	int GetLegendHeight() const;
	QRect GetLineDrawRect() const;
	QRect GetMinimizeRect() const;
	QRect GetMaximizeRect() const;
	QRect GetLockRect() const;
	QRect GetIncreaseRect() const;
	QRect GetScaleRect() const;
	QRect GetDecreaseRect() const;
	QRect GetOffsetRightRect() const;
	QRect GetOffsetLeftRect() const;
	
	void SetPointValue(uint32 lineId, uint32 pointId, Vector2 value, bool deleteSamePoints);
	
	void AddPoint(uint32 lineId, const Vector2& point);
	bool DeletePoint(uint32 lineId, uint32 pointId);
	
	float32 GetYFromX(uint32 lineId, float32 x);
	
	void GraphRectClick(QMouseEvent *event);
	
	void UpdateLimits();
	
	void GetClickedPoint(const QPoint& point, int32& pointId, int32& lineId) const;
	void UpdateSizePolicy();
	
	void ChangePointValueDialog(uint32 pointId, int32 lineId);
	
	void PostAddLine();
	
	QString float2QString(float32 value) const;
	
	int32 GetIntValue(float32 value) const;

	void PerformZoom(float newScale);

	void PerformOffset(int value);
	void DrawUITriangle(QPainter& painter, const QRect& rect, int rotateDegree);

	void GetCrossingPoint(const QPoint& firstPoint, const QPoint& secondPoint, QPoint & leftBorderCrossPoint, QPoint & rightBorderCrossPoint);

	enum ePositionRelativelyToDrawRect
	{
		POSITION_LEFT,
		POSITION_RIGHT,
		POSITION_INSIDE
	};
	ePositionRelativelyToDrawRect GetPointPositionFromDrawingRect(QPoint point); 

private:
	QPoint mouseStartPos;
	
	float32 minValue;
	float32 maxValue;
	float32 minTime;
	float32 maxTime;
	float32 generalMinTime;
	float32 generalMaxTime;
	float32 minValueLimit;
	float32 maxValueLimit;
	
	int32 selectedPoint;
	int32 selectedLine;
	int32 drawLine;
	
	bool isLockEnable;
	bool isLocked;
	bool isInteger;
	
	enum eGridStyle
	{
		GRID_STYLE_ALL_POSITION,
		GRID_STYLE_LIMITS
	};
	eGridStyle gridStyle;
	
	enum eSizeState
	{
		SIZE_STATE_NORMAL,
		SIZE_STATE_MINIMIZED,
		SIZE_STATE_DOUBLE
	};
	eSizeState sizeState;
	bool updateSizeState;
	bool aliasLinePoint;
	bool allowDeleteLine;

	typedef struct{
		LOGIC_POINTS line;
		QColor color;
		QString legend;
	}LINE;
	typedef Map<uint32, LINE> LINES_MAP;
	LINES_MAP lines;
	
	QBrush backgroundBrush;
	
	Vector2	newPoint;

	bool	isCtrlPressed;

	float32	scale;
	float32	initialTimeInterval;
};

class SetPointValueDlg: public QDialog
{
	Q_OBJECT
	
public:
	explicit SetPointValueDlg(float32 time, float32 minTime, float32 maxTime, float32 value, float32 minValue, float32 maxValue, QWidget *parent = 0, bool integer = false);
	
	float32 GetTime() const;
	float32 GetValue() const;

private:
	bool isInteger;

	QDoubleSpinBox* timeSpin;
	QDoubleSpinBox* valueSpin;
	QSpinBox* valueSpinInt;
};

#endif // TIMELINE_H
