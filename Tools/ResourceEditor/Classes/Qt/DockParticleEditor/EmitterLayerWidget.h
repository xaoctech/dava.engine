//
//  ParticleEditorWidget.h
//  ResourceEditorQt
//
//  Created by adebt on 11/26/12.
//
//

#ifndef __ResourceEditorQt__EmitterLayerWidget__
#define __ResourceEditorQt__EmitterLayerWidget__

#include <DAVAEngine.h>
#include "TimeLineWidget.h"
#include "GradientPickerWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>

using namespace DAVA;

class EmitterLayerWidget: public QWidget
{
    Q_OBJECT
    
public:
    explicit EmitterLayerWidget(QWidget *parent = 0);
    ~EmitterLayerWidget();

	void Init(ParticleEmitter* emitter, ParticleLayer* layer, bool updateMinimized);
	
protected slots:
	void OnValueChanged();
	void OnSpriteBtn();
	
private:
	void InitWidget(QWidget* );
	
private:
	ParticleEmitter* emitter;
	ParticleLayer* layer;
	QVBoxLayout* mainBox;
	
	QCheckBox* enableCheckBox;
	Sprite* sprite;
	QLabel* spriteLabel;
	//QLabel* spritePathLabel;
	QLineEdit* spritePathLabel;
	TimeLineWidget* lifeTimeLine;
	TimeLineWidget* numberTimeLine;
	TimeLineWidget* sizeTimeLine;
	TimeLineWidget* sizeVariationTimeLine;
	TimeLineWidget* sizeOverLifeTimeLine;
	TimeLineWidget* velocityTimeLine;
	TimeLineWidget* spinTimeLine;
	TimeLineWidget* motionTimeLine;
	TimeLineWidget* bounceTimeLine;
	TimeLineWidget* alphaOverLifeTimeLine;
	TimeLineWidget* frameOverLifeTimeLine;
	GradientPickerWidget* colorRandomGradient;
	GradientPickerWidget* colorOverLifeGradient;
	QDoubleSpinBox* alignToMotionSpin;
	QDoubleSpinBox* startTimeSpin;
	QDoubleSpinBox* endTimeSpin;
	
	bool blockSignals;
};

#endif /* defined(__ResourceEditorQt__EmitterLayerWidget__) */
