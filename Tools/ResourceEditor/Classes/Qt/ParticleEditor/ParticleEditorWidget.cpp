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
		return;
	
	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;

	emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(this);
	emitterPropertiesWidget->Init(emitter, true);
	setWidget(emitterPropertiesWidget);
}

void ParticleEditorWidget::OnLayerSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer)
{
	DeleteOldWidget();
	
	if (!emitterNode || !layer)
		return;

	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;

	emitterLayerWidget = new EmitterLayerWidget(this);
	emitterLayerWidget->Init(emitter, layer, true);
	setWidget(emitterLayerWidget);
}

void ParticleEditorWidget::OnForceSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer, int32 forceIndex)
{
	DeleteOldWidget();
	
	if (!emitterNode || !layer)
		return;
	
	ParticleEmitter* emitter = emitterNode->GetEmitter();
	if (!emitter)
		return;
	
	
	layerForceWidget = new LayerForceWidget(this);
	layerForceWidget->Init(emitter, layer, forceIndex, true);
	setWidget(layerForceWidget);
}