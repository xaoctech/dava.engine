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
#include "ScrollZoomWidget.h"

using namespace DAVA;

class ParticleTimeLineWidget : public ScrollZoomWidget
{
	Q_OBJECT
	
public:
	explicit ParticleTimeLineWidget(QWidget *parent = 0);
	~ParticleTimeLineWidget();
	
	void Init(float32 minTime, float32 maxTime);
	
signals:
	void ChangeVisible(bool visible);
	
protected slots:
	void OnNodeSelected(Entity* node);
	void OnEffectNodeSelected(Entity* node);
	void OnLayerSelected(Entity* node, ParticleLayer* layer);
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
	
	void AddLayerLine(uint32 layerLineID, float32 minTime, float32 maxTime,
					  const QColor& layerColor, ParticleLayer* layer);
	void AddLine(uint32 lineId, float32 startTime, float32 endTime, const QColor& color, const QString& legend, ParticleLayer* layer);

	void OnValueChanged(int lineId);
	void UpdateSizePolicy();
	
	// Handle situation when the Particle Emitter Node is selected (including
	// case when separate Layer node is selected.
	void HandleNodeSelected(Entity* node, ParticleLayer* layer);
	
	virtual QRect GetSliderRect() const;
	virtual QRect GetIncreaseRect() const;
	virtual QRect GetScaleRect() const;
	virtual QRect GetDecreaseRect() const;
	

private:
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
