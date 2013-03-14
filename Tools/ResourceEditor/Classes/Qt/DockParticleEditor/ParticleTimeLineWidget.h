//
//  ParticleTimeLineWidget.h
//  ResourceEditorQt
//
//  Created by adebt on 1/2/13.
//
//

#ifndef __ResourceEditorQt__ParticleTimeLineWidget__
#define __ResourceEditorQt__ParticleTimeLineWidget__

#include <QWidget>
#include <QDialog>
#include <QDoubleSpinBox>
#include <DAVAEngine.h>

using namespace DAVA;

class ParticleTimeLineWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit ParticleTimeLineWidget(QWidget *parent = 0);
	~ParticleTimeLineWidget();
	
	void Init(float32 minTime, float32 maxTime);
	
signals:
	void ValueChanged();
	void ChangeVisible(bool visible);
	
protected slots:
	void OnNodeSelected(Entity* node);
	void OnEffectNodeSelected(Entity* node);
	void OnUpdate();
	
protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	
private:
	bool GetLineRect(uint32 id, QRect& startPoint, QRect& endPoint) const;
	QRect GetGraphRect() const;
	QPoint GetPoint(const QPoint&) const;
	void AddLine(uint32 lineId, float32 startTime, float32 endTime, const QColor& color, const QString& legend, ParticleLayer* layer);
	void OnValueChanged(int lineId);
	void UpdateSizePolicy();
	QString float2QString(float32 value) const;
	
private:
	QBrush backgroundBrush;
	float32 minTime;
	float32 maxTime;
	QFont nameFont;
	
	struct LINE
	{
		float32 startTime;
		float32 endTime;
		QColor color;
		QString legend;
		ParticleLayer* layer;
	};
	
	typedef DAVA::Map<uint32, LINE> LINE_MAP;
	LINE_MAP lines;
	
	QPoint selectedPoint;
	Entity* emitterNode;
	Entity* effectNode;
	
	enum eGridStyle
	{
		GRID_STYLE_ALL_POSITION,
		GRID_STYLE_LIMITS
	};
	eGridStyle gridStyle;
	
	class SetPointValueDlg: public QDialog
	{
		//Q_OBJECT
		
	public:
		explicit SetPointValueDlg(float32 value, float32 minValue, float32 maxValue, QWidget *parent = 0);
		
		float32 GetValue() const;
		
	private:
		QDoubleSpinBox* valueSpin;
	};
};

#endif /* defined(__ResourceEditorQt__ParticleTimeLineWidget__) */
