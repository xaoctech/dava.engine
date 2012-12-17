#ifndef __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__
#define __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>
#include "GradientPickerWidget.h"
#include "TimeLineWidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>

class ParticleEmitterPropertiesWidget: public QWidget
{
	Q_OBJECT
	
public:
	explicit ParticleEmitterPropertiesWidget(QWidget* parent = 0);
	~ParticleEmitterPropertiesWidget();

	void Init(DAVA::ParticleEmitter* emitter, bool updateMinimize);

protected:
	
signals:
	
public slots:
	void OnValueChanged();

private:
	QVBoxLayout* mainLayout;

	ParticleEmitter* emitter;

	QComboBox* emitterType;
	TimeLineWidget* emitterEmissionAngle;
	TimeLineWidget* emitterEmissionRange;
	TimeLineWidget* emitterEmissionVector;
	TimeLineWidget* emitterRadius;
	TimeLineWidget* emitterSize;
	QDoubleSpinBox* emitterLife;
	GradientPickerWidget* emitterColorWidget;
	
	bool blockSignals;
	
	void InitWidget(QWidget* widget, bool connectWidget = true);
};

#endif // __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__