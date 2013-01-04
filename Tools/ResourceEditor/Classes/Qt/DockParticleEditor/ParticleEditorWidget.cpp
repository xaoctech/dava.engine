//
//  ParticleEditorWidget.cpp
//  ResourceEditorQt
//
//  Created by adebt on 11/26/12.
//
//

#include "ParticleEditorWidget.h"
#include "EmitterLayerWidget.h"
#include "LayerForceWidget.h"
#include "ParticlesEditorController.h"
#include "ui_mainwindow.h"

ParticleEditorWidget::ParticleEditorWidget(QWidget *parent/* = 0*/) :
	QScrollArea(parent)
{
	setWidgetResizable(true);
	
	emitterLayerWidget = NULL;
	layerForceWidget = NULL;
	emitterPropertiesWidget = NULL;
	
	connect(ParticlesEditorController::Instance(),
			SIGNAL(EmitterSelected(ParticleEmitterNode*)),
			this,
			SLOT(OnEmitterSelected(ParticleEmitterNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(LayerSelected(ParticleEmitterNode*, ParticleLayer*)),
			this,
			SLOT(OnLayerSelected(ParticleEmitterNode*, ParticleLayer*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(ForceSelected(ParticleEmitterNode*, ParticleLayer*, int32)),
			this,
			SLOT(OnForceSelected(ParticleEmitterNode*, ParticleLayer*, int32)));
}

ParticleEditorWidget::~ParticleEditorWidget()
{
	DeleteOldWidget();
}

void ParticleEditorWidget::DeleteOldWidget()
{
	SAFE_DELETE(emitterLayerWidget);
	SAFE_DELETE(layerForceWidget);
	SAFE_DELETE(emitterPropertiesWidget);
}

void ParticleEditorWidget::OnEmitterSelected(ParticleEmitterNode* emitterNode)
{
	DeleteOldWidget();
	
	if (!emitterNode)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;

	emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(this);
	emitterPropertiesWidget->Init(emitter, true);
	setWidget(emitterPropertiesWidget);
	connect(emitterPropertiesWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void ParticleEditorWidget::OnLayerSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer)
{
	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}

	emit ChangeVisible(true);
	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;

	emitterLayerWidget = new EmitterLayerWidget(this);
	emitterLayerWidget->Init(emitter, layer, true);
	setWidget(emitterLayerWidget);
	connect(emitterLayerWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void ParticleEditorWidget::OnForceSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer, int32 forceIndex)
{
	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;
	
	layerForceWidget = new LayerForceWidget(this);
	layerForceWidget->Init(emitter, layer, forceIndex, true);
	setWidget(layerForceWidget);
	connect(layerForceWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void ParticleEditorWidget::OnUpdate()
{
	if (emitterLayerWidget)
		emitterLayerWidget->Update();
	if (layerForceWidget)
		layerForceWidget->Update();
	if (emitterPropertiesWidget)
		emitterPropertiesWidget->Update();
}

void ParticleEditorWidget::OnValueChanged()
{
	emit ValueChanged();
}
