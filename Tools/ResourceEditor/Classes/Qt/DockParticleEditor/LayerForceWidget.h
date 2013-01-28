//
//  LayerForceWidget.h
//  ResourceEditorQt
//
//  Created by adebt on 12/10/12.
//
//

#ifndef __ResourceEditorQt__LayerForceWidget__
#define __ResourceEditorQt__LayerForceWidget__

#include <DAVAEngine.h>

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

using namespace DAVA;

class TimeLineWidget;
class QVBoxLayout;

class LayerForceWidget: public QWidget, public BaseParticleEditorContentWidget
{
    Q_OBJECT
    
public:
    explicit LayerForceWidget(QWidget *parent = 0);
    ~LayerForceWidget();
	
	void Init(ParticleEmitter* emitter, ParticleLayer* layer, uint32 forceIndex, bool updateMinimized);
	void Update();

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

signals:
	void ValueChanged();
	
protected slots:
	void OnValueChanged();
	
protected:
	void InitWidget(QWidget* widget);
	
private:
	QVBoxLayout* mainBox;
	ParticleLayer* layer;
	int32 forceIndex;
	
	TimeLineWidget* forceTimeLine;
	TimeLineWidget* forceVariationTimeLine;
	TimeLineWidget* forceOverLifeTimeLine;
	
	bool blockSignals;
};

#endif /* defined(__ResourceEditorQt__LayerForceWidget__) */
