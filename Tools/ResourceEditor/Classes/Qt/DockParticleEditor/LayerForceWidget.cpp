//
//  LayerForceWidget.cpp
//  ResourceEditorQt
//
//  Created by adebt on 12/10/12.
//
//

#include "LayerForceWidget.h"
#include "TimeLineWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QSizePolicy>

LayerForceWidget::LayerForceWidget(QWidget *parent):
	QWidget(parent)
{
	mainBox = new QVBoxLayout;
	this->setLayout(mainBox);
	
	forceTimeLine = new TimeLineWidget(this);
	InitWidget(forceTimeLine);
	forceVariationTimeLine = new TimeLineWidget(this);
	InitWidget(forceVariationTimeLine);
	forceOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(forceOverLifeTimeLine);
	
	blockSignals = false;
}

LayerForceWidget::~LayerForceWidget()
{
	
}


void LayerForceWidget::InitWidget(QWidget* widget)
{
	mainBox->addWidget(widget);
	connect(widget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void LayerForceWidget::Init(ParticleEmitter* emitter, ParticleLayer* layer, uint32 forceIndex, bool updateMinimized)
{	
	if (!layer ||
		layer->forces.size() <= forceIndex ||
		layer->forcesOverLife.size() <= forceIndex ||
		layer->forcesVariation.size() <= forceIndex)
		return;
	
	this->emitter = emitter;
	this->layer = layer;
	this->forceIndex = forceIndex;
	
	blockSignals = true;
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);

	Vector<QColor> colors;
	colors.push_back(Qt::blue); colors.push_back(Qt::green); colors.push_back(Qt::red);
	Vector<QString> legends;
	legends.push_back("Force X"); legends.push_back("Y"); legends.push_back("Z");
	forceTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceTimeLine->AddLines(PropLineWrapper<Vector3>(layer->forces[forceIndex]).GetProps(), colors, legends);

	legends.clear();
	legends.push_back("Force variable X"); legends.push_back("Y"); legends.push_back("Z");
	forceVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceVariationTimeLine->AddLines(PropLineWrapper<Vector3>(layer->forcesVariation[forceIndex]).GetProps(), colors, legends);

	forceOverLifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->forcesOverLife[forceIndex]).GetProps(), Qt::blue, "Forces over life");

	blockSignals = false;
}

void LayerForceWidget::OnValueChanged()
{
	if (blockSignals)
		return;
	
	PropLineWrapper<Vector3> propForce;
	forceTimeLine->GetValues(propForce.GetPropsPtr());
	PropLineWrapper<Vector3> propForceVariable;
	forceVariationTimeLine->GetValues(propForceVariable.GetPropsPtr());
	PropLineWrapper<float32> propForceOverLife;
	forceOverLifeTimeLine->GetValue(0, propForceOverLife.GetPropsPtr());

	CommandUpdateParticleLayerForce* updateForceCmd = new CommandUpdateParticleLayerForce(layer, forceIndex);
	updateForceCmd->Init(propForce.GetPropLine(),
						 propForceVariable.GetPropLine(),
						 propForceOverLife.GetPropLine());
	
	CommandsManager::Instance()->Execute(updateForceCmd);
	SafeRelease(updateForceCmd);
	
	Init(emitter, layer, forceIndex, false);
}