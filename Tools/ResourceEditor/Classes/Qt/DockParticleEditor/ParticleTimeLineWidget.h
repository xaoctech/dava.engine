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
#include <QLabel>
#include <QTimer>

#include <DAVAEngine.h>

using namespace DAVA;
class ParticlesCountWidget;
class ParticleTimeLineWidget : public QWidget
{
	Q_OBJECT
	friend class ParticlesCountWidget;

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
	void OnLayerSelected(Entity* node, ParticleLayer* layer);
	void OnUpdate();

	void OnUpdateParticlesCountNeeded();

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
	QString float2QString(float32 value) const;
	
	// Handle situation when the Particle Emitter Node is selected (including
	// case when separate Layer node is selected.
	void HandleNodeSelected(Entity* node, ParticleLayer* layer);

private:
	struct LINE
	{
		float32 startTime;
		float32 endTime;
		QColor color;
		QString legend;
		ParticleLayer* layer;
	};

	void UpdateParticlesCountPosition();
	void UpdateParticlesCountValues();

	// Get the width/height for particle counter label.
	void GetParticlesCountWidthHeight(const LINE& line, int32& width, int32& height);

	typedef DAVA::Map<uint32, LINE> LINE_MAP;
	LINE_MAP lines;

	QBrush backgroundBrush;
	float32 minTime;
	float32 maxTime;
	QFont nameFont;
	
	QPoint selectedPoint;
	Entity* emitterNode;
	Entity* effectNode;
	
	ParticlesCountWidget* countWidget;
	QTimer updateTimer;

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

////////////////////////////////////////////////////////////////////////////////////
// A specific class to display the per-layer counters.
class ParticlesCountWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ParticlesCountWidget(const ParticleTimeLineWidget* timeLineWidget,
								  QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);
	
	// Get the string representation of active particles count for the current layer.
	QString GetActiveParticlesCount(const ParticleTimeLineWidget::LINE& line);

private:
	const ParticleTimeLineWidget* timeLineWidget;
};

#endif /* defined(__ResourceEditorQt__ParticleTimeLineWidget__) */
