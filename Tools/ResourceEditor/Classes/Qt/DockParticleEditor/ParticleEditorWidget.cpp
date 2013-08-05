/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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
	effectPropertiesWidget = NULL;

	// New signals for Scene Tree.
	connect(SceneSignals::Instance(),
			SIGNAL(EffectSelected(SceneEditor2*, DAVA::Entity*)),
			this,
			SLOT(OnEffectSelectedFromSceneTree(SceneEditor2*, DAVA::Entity*)));
	connect(SceneSignals::Instance(),
			SIGNAL(EmitterSelected(SceneEditor2*, DAVA::Entity*)),
			this,
			SLOT(OnEmitterSelectedFromSceneTree(SceneEditor2*, DAVA::Entity*)));
	connect(SceneSignals::Instance(),
			SIGNAL(LayerSelected(SceneEditor2*, DAVA::ParticleLayer*, bool)),
			this,
			SLOT(OnLayerSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleLayer*, bool)));
	connect(SceneSignals::Instance(),
			SIGNAL(ForceSelected(SceneEditor2*, DAVA::ParticleLayer*, DAVA::int32)),
			this,
			SLOT(OnForceSelectedFromSceneTree(SceneEditor2*, DAVA::ParticleLayer*, DAVA::int32)));
	
	// Get the notification about changes in Particle Editor items.
	connect(SceneSignals::Instance(),
			SIGNAL(ParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)),
			this,
			SLOT(OnParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)));
	
	connect(SceneSignals::Instance(),
			SIGNAL(ParticleEmitterLoaded(SceneEditor2*, DAVA::ParticleEmitter*)),
			this,
			SLOT(OnParticleEmitterLoaded(SceneEditor2*, DAVA::ParticleEmitter*)));
	connect(SceneSignals::Instance(),
			SIGNAL(ParticleEmitterSaved(SceneEditor2*, DAVA::ParticleEmitter*)),
			this,
			SLOT(OnParticleEmitterSaved(SceneEditor2*, DAVA::ParticleEmitter*)));
}

ParticleEditorWidget::~ParticleEditorWidget()
{
	DeleteOldWidget();
}

void ParticleEditorWidget::DeleteOldWidget()
{
	DAVA::uint64 tt = DAVA::SystemTimer::Instance()->AbsoluteMS();

	SAFE_DELETE(emitterLayerWidget);
	SAFE_DELETE(layerForceWidget);
	SAFE_DELETE(emitterPropertiesWidget);
	SAFE_DELETE(effectPropertiesWidget);

	printf("delete %lld\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - tt);
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
	// Update the particle editor widgets when the value on the emitter layer is changed.
	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::UpdateParticleEditorWidgets()
{
	if (emitterPropertiesWidget && emitterPropertiesWidget->GetEmitter())
	{
		UpdateVisibleTimelinesForParticleEmitter();
		return;
	}
	
	if (emitterLayerWidget && emitterLayerWidget->GetLayer())
	{
		UpdateWidgetsForLayer();
		return;
	}
}

void ParticleEditorWidget::UpdateVisibleTimelinesForParticleEmitter()
{
	// Safety check.
	if (!emitterPropertiesWidget || !emitterPropertiesWidget->GetEmitter())
	{
		return;
	}

	// Update the visibility of particular timelines based on the emitter type.
	bool radiusTimeLineVisible = false;
	bool sizeTimeLineVisible = false;

	switch (emitterPropertiesWidget->GetEmitter()->emitterType)
	{
		case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
		case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
		case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
		{
			radiusTimeLineVisible = true;
			break;
		}
			
		case DAVA::ParticleEmitter::EMITTER_RECT:
		{
			sizeTimeLineVisible = true;
		}
		
		default:
		{
			break;
		}
	}
	
	emitterPropertiesWidget->GetEmitterRadiusTimeline()->setVisible(radiusTimeLineVisible);
	emitterPropertiesWidget->GetEmitterSizeTimeline()->setVisible(sizeTimeLineVisible);
}

void ParticleEditorWidget::UpdateWidgetsForLayer()
{
	if (!emitterLayerWidget || !emitterLayerWidget->GetLayer())
	{
		return;
	}
	
	bool isSuperemitter = (emitterLayerWidget->GetLayer()->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES);
	emitterLayerWidget->SetSuperemitterMode(isSuperemitter);
}

void ParticleEditorWidget::OnEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::Entity* emitterNode)
{
	if (!emitterNode)
	{
		// This means nothing Particle Emitter-related is selected.
		OnNodeDeselected(NULL);
	}
	
	// NULL is accepted here too.
	ParticleEmitter* emitter = GetEmitter(emitterNode);
	HandleEmitterSelected(scene, emitter, false);
}

void ParticleEditorWidget::HandleEmitterSelected(SceneEditor2* scene, DAVA::ParticleEmitter* emitter, bool forceUpdate)
{
	if (emitter &&
		emitterPropertiesWidget &&
		(!forceUpdate && (emitterPropertiesWidget->GetEmitter() == emitter)))
	{
		return;
	}

	DeleteOldWidget();
	
	if (!emitter)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	emitterPropertiesWidget = new ParticleEmitterPropertiesWidget(scene, this);
	emitterPropertiesWidget->Init(emitter, true);

	setWidget(emitterPropertiesWidget);
	connect(emitterPropertiesWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	// Yuri Coder, 2013/07/05. Yuri Coder - this storage is outdated.
	/*
	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterPropertiesWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
	 */
	
	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnEffectSelectedFromSceneTree(SceneEditor2* scene, DAVA::Entity* effectNode)
{
	ParticleEffectComponent* effect = NULL;
	if (effectNode)
	{
		effect = cast_if_equal<ParticleEffectComponent*>(effectNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		if (!effect)
		{
			return;
		}
		
		if (effectPropertiesWidget && effectPropertiesWidget->GetEffect() == effect)
		{
			return;
		}
	}
	
	DeleteOldWidget();
	if (!effect)
	{
		emit ChangeVisible(false);
		return;
	}
	
	emit ChangeVisible(true);
	effectPropertiesWidget = new ParticleEffectPropertiesWidget(scene, this);
	effectPropertiesWidget->Init(effect);
	
	setWidget(effectPropertiesWidget);
}

void ParticleEditorWidget::OnLayerSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleLayer* layer, bool forceRefresh)
{
	ParticleEmitter* emitter = layer->GetEmitter();

	if (!forceRefresh && emitterLayerWidget &&
		emitterLayerWidget->GetLayer() == layer &&
		emitterLayerWidget->GetEmitter() == emitter &&
		!forceRefresh)
	{
		return;
	}

	DeleteOldWidget();

	if (!emitter || !layer)
	{
		emit ChangeVisible(false);
		return;
	}

	emit ChangeVisible(true);

	DAVA::uint64 tt = DAVA::SystemTimer::Instance()->AbsoluteMS();

	emitterLayerWidget = new EmitterLayerWidget(scene, this);
	emitterLayerWidget->Init(emitter, layer, true);

	setWidget(emitterLayerWidget);
	connect(emitterLayerWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	printf("create: %lld\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - tt);

	// TODO: Yuri Coder, 2013/07/22. This code does not work now.
	/*
	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		emitterLayerWidget->RestoreVisualState(stateProps);
	
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
	 */

	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnForceSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::int32 forceIndex)
{
	ParticleEmitter* emitter = layer->GetEmitter();
	if (layerForceWidget &&
		layerForceWidget->GetLayer() == layer &&
		layerForceWidget->GetForceIndex() == forceIndex &&
		layerForceWidget->GetEmitter() == emitter)
	{
		return;
	}
	
	DeleteOldWidget();
	
	if (!emitter || !layer)
	{
		emit ChangeVisible(false);
		return;
	}

	emit ChangeVisible(true);

	layerForceWidget = new LayerForceWidget(scene, this);
	layerForceWidget->Init(emitter, layer, forceIndex, true);
	
	setWidget(layerForceWidget);
	connect(layerForceWidget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));

	// TODO: Yuri Coder, 2013/07/22. This code does not work now.
	/*
	if (editorNode)
	{
		KeyedArchive* stateProps = editorNode->GetExtraData();
		layerForceWidget->RestoreVisualState(stateProps);
		
		int32 scrollValue = stateProps->GetInt32("EDITOR_SCROLL_VALUE", 0);
		verticalScrollBar()->setValue(scrollValue);
	}
	 */

	UpdateParticleEditorWidgets();
}

void ParticleEditorWidget::OnParticleLayerValueChanged(SceneEditor2* /*scene*/, DAVA::ParticleLayer* layer)
{
	if (!emitterLayerWidget || emitterLayerWidget->GetLayer() != layer)
	{
		return;
	}
	
	// Notify the Emitter Layer widget about its inner layer value is changed and
	// the widget needs to be resynchronized with its values.
	emitterLayerWidget->OnLayerValueChanged();
}

void ParticleEditorWidget::OnParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
{
	// Handle in the same way emitter is selected to update the values. However
	// cause widget to be force updated.
	HandleEmitterSelected(scene, emitter, true);
}

void ParticleEditorWidget::OnParticleEmitterSaved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter)
{
	// Handle in the same way emitter is selected to update the values. However
	// cause widget to be force updated.
	HandleEmitterSelected(scene, emitter, true);
}

