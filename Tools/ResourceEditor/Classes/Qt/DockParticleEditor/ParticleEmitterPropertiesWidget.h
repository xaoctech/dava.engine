#ifndef __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__
#define __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>
#include "GradientPickerWidget.h"
#include "TimeLineWidget.h"
#include "BaseParticleEditorContentWidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>

class ParticleEmitterPropertiesWidget: public QWidget, public BaseParticleEditorContentWidget
{
	Q_OBJECT
	
public:
	explicit ParticleEmitterPropertiesWidget(QWidget* parent = 0);
	~ParticleEmitterPropertiesWidget();

	void Init(DAVA::ParticleEmitter* emitter, bool updateMinimize);
	void Update();
	
	virtual bool eventFilter( QObject * o, QEvent * e );

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

signals:
	void ValueChanged();
	
public slots:
	void OnValueChanged();

private:
	QVBoxLayout* mainLayout;

	ParticleEmitter* emitter;

	QLineEdit* emitterYamlPath;
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