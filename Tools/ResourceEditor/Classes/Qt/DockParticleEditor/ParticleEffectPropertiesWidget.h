//
//  ParticleEffectPropertiesWidget.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 4/11/13.
//
//

#ifndef __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__
#define __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>

class ParticleEffectPropertiesWidget: public QWidget, public BaseParticleEditorContentWidget
{
	Q_OBJECT
	
public:
	explicit ParticleEffectPropertiesWidget(QWidget* parent = 0);
	~ParticleEffectPropertiesWidget();

	void Init(DAVA::ParticleEffectComponent* effect);
	ParticleEffectComponent* GetEffect() {return particleEffect;};

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

public slots:
	void OnValueChanged();
	
protected:
	void InitWidget(QWidget* widget, bool connectWidget = true);
	void UpdatePlaybackSpeedLabel();

private:
	ParticleEffectComponent* particleEffect;

	QVBoxLayout* mainLayout;

	QLabel* effectPlaybackSpeedLabel;
	QSlider* effectPlaybackSpeed;
	
	bool blockSignals;
};


#endif /* defined(__PARTICLE_EFFECT_PROPERTIES_WIDGET__H__) */
