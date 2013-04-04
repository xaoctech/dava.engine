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
#include "BaseParticleEditorContentWidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>

using namespace DAVA;

class EmitterLayerWidget: public QWidget, public BaseParticleEditorContentWidget
{
    Q_OBJECT
    
public:
    explicit EmitterLayerWidget(QWidget *parent = 0);
    ~EmitterLayerWidget();

	void Init(ParticleEmitter* emitter, ParticleLayer* layer, bool updateMinimized);
	ParticleLayer* GetLayer() const {return layer;};
	void Update();
	
	virtual bool eventFilter(QObject *, QEvent *);

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

signals:
	void ValueChanged();
	
protected slots:
	void OnValueChanged();
	void OnSpriteBtn();
	void OnSpritePathChanged(const QString& text);
	
private:
	void InitWidget(QWidget* );
	void UpdateTooltip();
	
	void FillLayerTypes();
	int32 LayerTypeToIndex(ParticleLayer::eType layerType);

private:
	ParticleLayer* layer;
	QVBoxLayout* mainBox;
	
	QLineEdit* layerNameLineEdit;
	QCheckBox* enableCheckBox;
	QCheckBox* additiveCheckBox;

	QLabel* layerTypeLabel;
	QComboBox* layerTypeComboBox;

	Sprite* sprite;
	QLabel* spriteLabel;
	QPushButton* spriteBtn;
	//QLabel* spritePathLabel;
	QLineEdit* spritePathLabel;
	TimeLineWidget* lifeTimeLine;
	TimeLineWidget* numberTimeLine;
	TimeLineWidget* sizeTimeLine;
	TimeLineWidget* sizeVariationTimeLine;
	TimeLineWidget* sizeOverLifeTimeLine;
	TimeLineWidget* velocityTimeLine;
	TimeLineWidget* velocityOverLifeTimeLine;
	TimeLineWidget* spinTimeLine;
	TimeLineWidget* spinOverLifeTimeLine;

	TimeLineWidget* alphaOverLifeTimeLine;
	QCheckBox* frameOverlifeCheckBox;
	QSpinBox* frameOverlifeFPSSpin;
	TimeLineWidget* angleTimeLine;
	GradientPickerWidget* colorRandomGradient;
	GradientPickerWidget* colorOverLifeGradient;

	QDoubleSpinBox* startTimeSpin;
	QDoubleSpinBox* endTimeSpin;
	
	bool blockSignals;

	struct LayerTypeMap
	{
		ParticleLayer::eType layerType;
		QString layerName;
	};

	static const LayerTypeMap layerTypeMap[];
};

#endif /* defined(__ResourceEditorQt__EmitterLayerWidget__) */
