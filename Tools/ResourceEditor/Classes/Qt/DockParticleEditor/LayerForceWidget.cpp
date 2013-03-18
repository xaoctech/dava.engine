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
	if (!layer || layer->forces.size() <= forceIndex)
	{
		return;
	}
	
	this->emitter = emitter;
	this->layer = layer;
	this->forceIndex = forceIndex;
	
	blockSignals = true;
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);
	ParticleForce* curForce = layer->forces[forceIndex];

	Vector<QColor> colors;
	colors.push_back(Qt::blue); colors.push_back(Qt::darkGreen); colors.push_back(Qt::red);
	Vector<QString> legends;
	legends.push_back("force x"); legends.push_back("force y"); legends.push_back("force z");
	forceTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceTimeLine->AddLines(PropLineWrapper<Vector3>(curForce->GetForce()).GetProps(), colors, legends);
	forceTimeLine->EnableLock(true);

	legends.clear();
	legends.push_back("force variable x"); legends.push_back("force variable y"); legends.push_back("force variable z");
	forceVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceVariationTimeLine->AddLines(PropLineWrapper<Vector3>(curForce->GetForceVariation()).GetProps(), colors, legends);
	forceVariationTimeLine->EnableLock(true);

	forceOverLifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true, false);
	forceOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(curForce->GetForceOverlife()).GetProps(), Qt::blue, "forces over life");

	blockSignals = false;
}

void LayerForceWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	forceTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_PROPS"));
	forceVariationTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_VARIATION_PROPS"));
	forceOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("FORCE_OVER_LIFE_PROPS"));
}

void LayerForceWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	forceTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_PROPS", props);

	props->DeleteAllKeys();
	forceVariationTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_VARIATION_PROPS", props);

	props->DeleteAllKeys();
	forceOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("FORCE_OVER_LIFE_PROPS", props);

	SafeRelease(props);
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

	CommandUpdateParticleForce* updateForceCmd = new CommandUpdateParticleForce(layer, forceIndex);
	updateForceCmd->Init(propForce.GetPropLine(),
						 propForceVariable.GetPropLine(),
						 propForceOverLife.GetPropLine());
	
	CommandsManager::Instance()->ExecuteAndRelease(updateForceCmd);

	Init(emitter, layer, forceIndex, false);
	emit ValueChanged();
}

void LayerForceWidget::Update()
{
	Init(emitter, layer, forceIndex, false);
}