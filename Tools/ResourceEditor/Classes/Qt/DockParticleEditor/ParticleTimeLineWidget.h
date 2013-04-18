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
#include "ScrollZoomWidget.h"

using namespace DAVA;

class ParticlesCountWidget;
class ParticlesAreaWidget;
class ParticleTimeLineWidget : public ScrollZoomWidget
{
	Q_OBJECT
	friend class ParticlesExtraInfoWidget;

public:
	explicit ParticleTimeLineWidget(QWidget *parent = 0);
	~ParticleTimeLineWidget();
	
	void Init(float32 minTime, float32 maxTime);

	struct LINE
	{
		float32 startTime;
		float32 endTime;
		QColor color;
		QString legend;
		ParticleLayer* layer;
	};
	typedef DAVA::Map<uint32, LINE> LINE_MAP;

signals:
	void ChangeVisible(bool visible);
	
protected slots:
	void OnNodeSelected(Entity* node);
	void OnEffectNodeSelected(Entity* node);
	void OnLayerSelected(Entity* node, ParticleLayer* layer);
	void OnUpdate();

	void OnUpdateLayersExtraInfoNeeded();

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

	void UpdateLayersExtraInfoPosition();
	void UpdateLayersExtraInfoValues();

	// Handle situation when the Particle Emitter Node is selected (including
	// case when separate Layer node is selected.
	void HandleNodeSelected(Entity* node, ParticleLayer* layer);
	
	virtual QRect GetSliderRect() const;
	virtual QRect GetIncreaseRect() const;
	virtual QRect GetScaleRect() const;
	virtual QRect GetDecreaseRect() const;

private:
	// Get the width/height for particle counter label.
	void GetParticlesCountWidthHeight(const LINE& line, int32& width, int32& height);

	LINE_MAP lines;
	QFont nameFont;
	
	QPoint selectedPoint;
	Entity* emitterNode;
	Entity* effectNode;
	
	QTimer updateTimer;
	ParticlesCountWidget* countWidget;
	ParticlesAreaWidget* areaWidget;

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
// A specific class to display the per-layer particles info.
const int32 EXTRA_INFO_LEFT_PADDING = 3;

class ParticlesExtraInfoWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit ParticlesExtraInfoWidget(const ParticleTimeLineWidget* timeLineWidget,
									  QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);
	
	// These methods are to be overriden for derived classes.
	// Get an extra information to be displayed near the line.
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line) {return QString();};

	// Get the extra info for the header/footer.
	virtual QString GetExtraInfoHeader() {return QString(); };
	virtual QString GetExtraInfoFooter() {return QString(); };

	// In case some information should be accumulated during the loop,
	// these methods are called just before the loop and just after it.
	virtual void OnBeforeGetExtraInfoLoop() {};
	virtual void OnAfterGetExtraInfoLoop() {};

	// The timeline widget being used.
	const ParticleTimeLineWidget* timeLineWidget;
};

// Particles Count information.
class ParticlesCountWidget : public ParticlesExtraInfoWidget
{
	Q_OBJECT
public:
	explicit ParticlesCountWidget(const ParticleTimeLineWidget* timeLineWidget,
									  QWidget *parent = 0);
protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line);

	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();

private:
	int32 totalParticlesCount;
};

// Particles Area information.
class ParticlesAreaWidget : public ParticlesExtraInfoWidget
{
	Q_OBJECT
public:
	explicit ParticlesAreaWidget(const ParticleTimeLineWidget* timeLineWidget,
								 QWidget *parent = 0);
protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
	
private:
	QString FormatFloat(float32 value);

	float32 totalParticlesArea;
};

#endif /* defined(__ResourceEditorQt__ParticleTimeLineWidget__) */
