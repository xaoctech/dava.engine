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
#include <QScrollBar>

ParticleEditorWidget::ParticleEditorWidget(QWidget *parent/* = 0*/) :
	QScrollArea(parent)
{
	setWidgetResizable(true);
	
	emitterLayerWidget = NULL;
	layerForceWidget = NULL;
	emitterPropertiesWidget = NULL;
	
	connect(ParticlesEditorController::Instance(),
			SIGNAL(EmitterSelected(Entity*, BaseParticleEditorNode*)),
			this,
			SLOT(OnEmitterSelected(Entity*, BaseParticleEditorNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(LayerSelected(Entity*, ParticleLayer*, BaseParticleEditorNode*, bool)),
			this,
			SLOT(OnLayerSelected(Entity*, ParticleLayer*, BaseParticleEditorNode*, bool)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(ForceSelected(Entity*, ParticleLayer*, int32, BaseParticleEditorNode*)),
			this,
			SLOT(OnForceSelected(Entity*, ParticleLayer*, int32, BaseParticleEditorNode*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(NodeDeselected(BaseParticleEditorNode*)),
			this,
			SLOT(OnNodeDeselected(BaseParticleEditorNode*)));
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

void ParticleEditorWidget::OnEmitterSelected(Entity* emitterNode, BaseParticleEditorNode* editorNode)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter = GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}

		if (emitterPropertiesWidget &&
			emitterPropertiesWidget->GetEmitter() == emitter)
			return;
	}
	
	DeleteOldWidget();
	
	if (!emitterNode)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	if (!emitter)
		return;

	emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(this);
	emitterPropertiesWidget->Init(emitter, true);

	setWidget(emitterPropertiesWidget);
	connect(emitterPropertiesWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterPropertiesWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
}

void ParticleEditorWidget::OnLayerSelected(Entity* emitterNode, ParticleLayer* layer, BaseParticleEditorNode* editorNode, bool forceRefresh)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter =  GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}
		if (!forceRefresh && emitterLayerWidget &&
			emitterLayerWidget->GetLayer() == layer &&
			emitterLayerWidget->GetEmitter() == emitter &&
			!forceRefresh)
			return;
	}

	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}

	emit ChangeVisible(true);
	if (!emitter)
		return;

	emitterLayerWidget = new EmitterLayerWidget(this);
	emitterLayerWidget->Init(emitter, layer, true);

	setWidget(emitterLayerWidget);
	connect(emitterLayerWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterLayerWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
}

void ParticleEditorWidget::OnForceSelected(Entity* emitterNode, ParticleLayer* layer, int32 forceIndex, BaseParticleEditorNode* editorNode)
{
	ParticleEmitter* emitter = NULL;
	if (emitterNode)
	{
		emitter =  GetEmitter(emitterNode);
		if (!emitter)
		{
			return;
		}
		if (layerForceWidget &&
			layerForceWidget->GetLayer() == layer &&
			layerForceWidget->GetForceIndex() == forceIndex &&
			layerForceWidget->GetEmitter() == emitter)
			return;
	}

	DeleteOldWidget();
	
	if (!emitterNode || !layer)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	if (!emitter)
		return;
	
	layerForceWidget = new LayerForceWidget(this);
	layerForceWidget->Init(emitter, layer, forceIndex, true);

	setWidget(layerForceWidget);
	connect(layerForceWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		layerForceWidget->RestoreVisualState(stateProps);

		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
}

void ParticleEditorWidget::OnNodeDeselected(BaseParticleEditorNode* particleEditorNode)
{
	if (!particleEditorNode)
		return;

	KeyedArchive* stateProps = particleEditorNode->GetExtraData();

	if (emitterLayerWidget)
		emitterLayerWidget->StoreVisualState(stateProps);
	if (layerForceWidget)
		layerForceWidget->StoreVisualState(stateProps);
	if (emitterPropertiesWidget)
		emitterPropertiesWidget->StoreVisualState(stateProps);

	stateProps->SetInt32("EDITOR_SCROLL_VALUE", verticalScrollBar()->value());
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
