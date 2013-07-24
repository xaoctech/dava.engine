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

#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneGraph.h"

#include "DockParticleEditor/ParticlesEditorController.h"
#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/InnerEmitterParticleEditorNode.h"

#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "SceneEditor/EditorSettings.h"
#include <QFileDialog>
#include <QString>

#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandUpdateEffect::CommandUpdateEffect(ParticleEffectComponent* particleEffect):
	CommandAction(CMDID_UPDATE_PARTICLE_EFFECT)
{
	this->particleEffect = particleEffect;
}

void CommandUpdateEffect::Init(float32 playbackSpeed, bool stopOnLoad)
{
	this->playbackSpeed = playbackSpeed;
	this->stopOnLoad = stopOnLoad;
}

void CommandUpdateEffect::Redo()
{
	DVASSERT(particleEffect);
	particleEffect->SetPlaybackSpeed(playbackSpeed);
	particleEffect->SetStopOnLoad(stopOnLoad);
}

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitter* emitter):
	CommandAction(CMDID_UPDATE_PARTICLE_EMITTER)
{
	this->emitter = emitter;
}

void CommandUpdateEmitter::Init(ParticleEmitter::eType emitterType,
								RefPtr<PropertyLine<float32> > emissionRange,
								RefPtr<PropertyLine<Vector3> > emissionVector,
								RefPtr<PropertyLine<float32> > radius,
								RefPtr<PropertyLine<Color> > colorOverLife,
								RefPtr<PropertyLine<Vector3> > size,
								float32 life,
								float32 playbackSpeed)
{
	this->emitterType = emitterType;
	this->emissionRange = emissionRange;
	this->emissionVector = emissionVector;
	this->radius = radius;
	this->colorOverLife = colorOverLife;
	this->size = size;
	this->life = life;
	this->playbackSpeed = playbackSpeed;
}

void CommandUpdateEmitter::Redo()
{
	DVASSERT(emitter);

	emitter->emitterType = emitterType;
	emitter->emissionRange = emissionRange;
	emitter->emissionVector = emissionVector;
	emitter->radius = radius;
	emitter->colorOverLife = colorOverLife;
	emitter->size = size;
	emitter->SetLifeTime(life);
	emitter->SetPlaybackSpeed(playbackSpeed);
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer) :
	CommandUpdateParticleLayerBase(CMDID_UPDATE_PARTICLE_LAYER)
{
	this->emitter = emitter;
	this->layer = layer;
}

void CommandUpdateParticleLayer::Init(const String& layerName,
									  ParticleLayer::eType layerType,
									  bool isDisabled,
									  bool additive,
									  bool isLong,
									  bool isLooped,
									  Sprite* sprite,
									  RefPtr< PropertyLine<float32> > life,
									  RefPtr< PropertyLine<float32> > lifeVariation,
									  RefPtr< PropertyLine<float32> > number,
									  RefPtr< PropertyLine<float32> > numberVariation,
									  RefPtr< PropertyLine<Vector2> > size,
									  RefPtr< PropertyLine<Vector2> > sizeVariation,
									  RefPtr< PropertyLine<Vector2> > sizeOverLife,
									  RefPtr< PropertyLine<float32> > velocity,
									  RefPtr< PropertyLine<float32> > velocityVariation,
									  RefPtr< PropertyLine<float32> > velocityOverLife,
									  RefPtr< PropertyLine<float32> > spin,
									  RefPtr< PropertyLine<float32> > spinVariation,
									  RefPtr< PropertyLine<float32> > spinOverLife,
									  RefPtr< PropertyLine<Color> > colorRandom,
									  RefPtr< PropertyLine<float32> > alphaOverLife,
									  RefPtr< PropertyLine<Color> > colorOverLife,
									  RefPtr< PropertyLine<float32> > angle,
									  RefPtr< PropertyLine<float32> > angleVariation,

									  float32 startTime,
									  float32 endTime,
									  bool frameOverLifeEnabled,
									  float32 frameOverLifeFPS,
									  
									  float32 pivotPointX,
									  float32 pivotPointY)
{
	this->layerName = layerName;
	this->layerType = layerType;
	this->isDisabled = isDisabled;
	this->additive = additive;
	this->isLooped = isLooped;
	this->isLong = isLong;
	this->sprite = sprite;
	this->life = life;
	this->lifeVariation = lifeVariation;
	this->number = number;
	this->numberVariation = numberVariation;
	this->size = size;
	this->sizeVariation = sizeVariation;
	this->sizeOverLife = sizeOverLife;
	this->velocity = velocity;
	this->velocityVariation = velocityVariation;
	this->velocityOverLife = velocityOverLife;
	this->spin = spin;
	this->spinVariation = spinVariation;
	this->spinOverLife = spinOverLife;

	this->colorRandom = colorRandom;
	this->alphaOverLife = alphaOverLife;
	this->colorOverLife = colorOverLife;
	this->frameOverLife = frameOverLife;
	this->angle = angle;
	this->angleVariation = angleVariation;

	this->startTime = startTime;
	this->endTime = endTime;
	this->frameOverLifeEnabled = frameOverLifeEnabled;
	this->frameOverLifeFPS = frameOverLifeFPS;
	
	this->pivotPointX = pivotPointX;
	this->pivotPointY = pivotPointY;
}


void CommandUpdateParticleLayer::Redo()
{
	layer->layerName = layerName;
	layer->SetDisabled(isDisabled);
	layer->SetAdditive(additive);
	layer->SetLong(isLong);
	layer->SetLooped(isLooped);
	layer->life = life;
	layer->lifeVariation = lifeVariation;
	layer->number = number;
	layer->numberVariation = numberVariation;
	layer->size = size;
	layer->sizeVariation = sizeVariation;
	layer->sizeOverLifeXY = sizeOverLife;
	layer->velocity = velocity;
	layer->velocityVariation = velocityVariation;
	layer->velocityOverLife = velocityOverLife;
	layer->spin = spin;
	layer->spinVariation = spinVariation;
	layer->spinOverLife = spinOverLife;

	layer->colorRandom = colorRandom;
	layer->alphaOverLife = alphaOverLife;
	layer->colorOverLife = colorOverLife;
	
	layer->frameOverLifeEnabled = frameOverLifeEnabled;
	layer->frameOverLifeFPS = frameOverLifeFPS;

	layer->angle = angle;
	layer->angleVariation = angleVariation;

	layer->startTime = startTime;
	layer->endTime = endTime;
	
	layer->SetPivotPoint(Vector2(pivotPointX, pivotPointY));

	// This code must be after layer->frameOverlife set call, since setSprite
	// may change the frames.
	if (layer->GetSprite() != sprite)
	{
		emitter->Stop();
		layer->SetSprite(sprite);
		emitter->Play();
	}
	
	// The same is for emitter type.
	if (layer->type != layerType)
	{
		emitter->Stop();
		layer->type = layerType;
		emitter->Play();
	}
	
	// "IsLong" flag.
	if (layer->IsLong() != isLong)
	{
		emitter->Stop();
		layer->SetLong(isLong);
		emitter->Play();
	}

	// In case we are switching to "SuperEmitter" type - need to create Inner Emitter
	// for the layer.
	if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES && !layer->GetInnerEmitter())
	{
		layer->CreateInnerEmitter();
	}

	// TODO: Yuri Coder, 2013/07/23. Refresh Particle Layer is needed for Superemitter.
	// Currently it is temporarily commented out but have to be recovered soon.
	// SceneDataManager::Instance()->RefreshParticlesLayer(layer);
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer) :
	CommandUpdateParticleLayerBase(CMDID_UPDATE_PARTILCE_LAYER_TIME)
{
	this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
	this->startTime = startTime;
	this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Redo()
{
	layer->startTime = startTime;
	layer->endTime = endTime;
}

CommandUpdateParticleLayerEnabled::CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled) :
	CommandUpdateParticleLayerBase(CMDID_UPDATE_PARTICLE_LAYER_ENABLED)
{
	this->layer = layer;
	this->isEnabled = isEnabled;
}

void CommandUpdateParticleLayerEnabled::Redo()
{
	if (this->layer)
	{
		this->layer->SetDisabled(!isEnabled);
		ParticlesEditorController::Instance()->RefreshSelectedNode(true);
	}
}

CommandUpdateParticleForce::CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId) :
	CommandAction(CMDID_UPDATE_PARTICLE_FORCE)
{
	this->layer = layer;
	this->forceId = forceId;
}

void CommandUpdateParticleForce::Init(RefPtr< PropertyLine<Vector3> > force,
									  RefPtr< PropertyLine<Vector3> > forcesVariation,
									RefPtr< PropertyLine<float32> > forcesOverLife)
{
	this->force = force;
	this->forcesVariation = forcesVariation;
	this->forcesOverLife = forcesOverLife;
}

void CommandUpdateParticleForce::Redo()
{
	layer->UpdateForce(forceId, force, forcesVariation, forcesOverLife);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandAddParticleEmitter::CommandAddParticleEmitter() :
    CommandAction(CMDID_ADD_PARTICLE_EMITTER)
{
}

void CommandAddParticleEmitter::Redo()
{
    // This command is done through Main Window to reuse the existing code.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->CreateParticleEmitterNode();
}


CommandStartStopParticleEffect::CommandStartStopParticleEffect(DAVA::Entity* effect, bool isStart) :
    CommandAction(CMDID_START_STOP_PARTICLE_EFFECT)
{
    this->isStart = isStart;
	this->effectEntity = effect;
}

void CommandStartStopParticleEffect::Redo()
{
	if (!effectEntity)
	{
		return;
	}

	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);

    if (this->isStart)
    {
        effectComponent->Start();
    }
    else
    {
        effectComponent->Stop();
    }
}

DAVA::Entity* CommandStartStopParticleEffect::GetEntity() const
{
	return this->effectEntity;
}

CommandRestartParticleEffect::CommandRestartParticleEffect(DAVA::Entity* effect) :
    CommandAction(CMDID_RESTART_PARTICLE_EFFECT)
{
	this->effectEntity = effect;
}

void CommandRestartParticleEffect::Redo()
{
	if (!effectEntity)
	{
		return;
	}
	
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
    effectComponent->Restart();
}

DAVA::Entity* CommandRestartParticleEffect::GetEntity() const
{
	return this->effectEntity;
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer() :
    CommandAction(CMDID_ADD_PARTICLE_EMITTER_LAYER)
{
}

void CommandAddParticleEmitterLayer::Redo()
{
    // Need to know selected Particle Emitter Layers Root.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode)
    {
        return;
    }

	ParticleEffectComponent * effectComponent = emitterNode->GetParticleEffectComponent();
	DVASSERT(effectComponent);
    effectComponent->Stop();
	
    // Lets select this node when the tree will be rebuilt.
    LayerParticleEditorNode* layerNode = ParticlesEditorController::Instance()->AddParticleLayerToNode(emitterNode);
    if (layerNode)
    {
        layerNode->SetMarkedToSelection(true);
    }

	effectComponent->Restart();

    // Update the scene graph.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer() :
    CommandAction(CMDID_REMOVE_PARTICLE_EMITTER_LAYER)
{
}

void CommandRemoveParticleEmitterLayer::Redo()
{
    // Need to know selected Layer Node to remove it.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode || !layerNode->GetRootNode())
    {
        return;
    }

    // Mark the "parent" Emitter Node to selection.
    layerNode->GetEmitterEditorNode()->SetMarkedToSelection(true);

	ParticleEffectComponent * effectComponent = layerNode->GetParticleEffectComponent();

	if (effectComponent)
	{
		effectComponent->Stop();
	}

    ParticlesEditorController::Instance()->RemoveParticleLayerNode(layerNode);

	if (effectComponent)
	{
		effectComponent->Restart();
	}
	
    // Update the scene graph.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer() :
	CommandAction(CMDID_CLONE_PARTICLE_EMITTER_LAYER)
{
}

void CommandCloneParticleEmitterLayer::Redo()
{
    // Need to know selected Layer Node to remove it.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode)
    {
        return;
    }
    
    LayerParticleEditorNode* clonedNode = ParticlesEditorController::Instance()->CloneParticleLayerNode(layerNode);
    if (clonedNode)
    {
        clonedNode->SetMarkedToSelection(true);
    }
    
    // Update the scene graph.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandAddParticleEmitterForce::CommandAddParticleEmitterForce() :
    CommandAction(CMDID_ADD_PARTICLE_EMITTER_FORCE)
{
}

void CommandAddParticleEmitterForce::Redo()
{
    // Need to know selected Layer Node to add the Force to.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode || !layerNode->GetRootNode())
    {
        return;
    }

	ParticleEffectComponent * effectComponent = layerNode->GetParticleEffectComponent();
	DVASSERT(effectComponent);
	effectComponent->Stop();
    ForceParticleEditorNode* forceNode = ParticlesEditorController::Instance()->AddParticleForceToNode(layerNode);
    if (forceNode)
    {
        forceNode->SetMarkedToSelection(true);
    }
	effectComponent->Restart();
	
    // Update the scene graph.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterForce::CommandRemoveParticleEmitterForce() :
    CommandAction(CMDID_REMOVE_PARTICLE_EMITTER_FORCE)
{
}

void CommandRemoveParticleEmitterForce::Redo()
{
    // Need to know selected Layer Node to add the Force to.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    ForceParticleEditorNode* forceNode = dynamic_cast<ForceParticleEditorNode*>(selectedNode);
    if (!forceNode || !forceNode->GetRootNode())
    {
        return;
    }

    // Mark the "parent" Layer Node to selection.
    forceNode->GetLayerEditorNode()->SetMarkedToSelection(true);

	
	ParticleEffectComponent * effectComponent = forceNode->GetParticleEffectComponent();
	if (effectComponent)
	{
		effectComponent->Stop();
	}

    ParticlesEditorController::Instance()->RemoveParticleForceNode(forceNode);

	if (effectComponent)
	{
		effectComponent->Restart();
	}

    // Update the scene graph.
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

// TODO! these commands use QT! Think about how to get rid of it.
/*
CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_LOAD_PARTICLE_EMITTER_FROM_YAML)
{
}

void CommandLoadParticleEmitterFromYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetEmitterNode())
    {
        return;
    }
    
    QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
	Logger::Debug("Project path: %s", projectPath.toStdString().c_str());
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                    projectPath, QString("YAML File (*.yaml)"));
	if (filePath.isEmpty())
    {
		return;
    }

    // In case this emitter already has Editor Nodes - remove them before loading.
    ParticlesEditorController::Instance()->CleanupParticleEmitterEditorNode(emitterNode);
    ParticleEmitter* emitter = emitterNode->GetParticleEmitter();

    if(!emitter)
    {
    	return;
    }

    emitter->LoadFromYaml(filePath.toStdString());

	// Perform the validation of the Yaml file loaded.
	String validationMessage;
	if (ParticlesEditorSceneDataHelper::ValidateParticleEmitter(emitter, validationMessage) == false)
	{
		ShowErrorDialog(validationMessage);
	}

	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(bool forceAskFilename) :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_PARTICLE_EMITTER_TO_YAML)
{
    this->forceAskFilename = forceAskFilename;
}

void CommandSaveParticleEmitterToYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetEmitterNode())
    {
        return;
    }

    ParticleEmitter * emitter = emitterNode->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

	FilePath yamlPath = emitter->GetConfigPath();
    if (this->forceAskFilename || yamlPath.IsEmpty() )
    {
        QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
 
        if (filePath.isEmpty())
        {
            return;
        }
        
        yamlPath = FilePath(filePath.toStdString());
    }

    emitter->SaveToYaml(yamlPath);
}

CommandLoadInnerEmitterFromYaml::CommandLoadInnerEmitterFromYaml() :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_LOAD_INNER_EMITTER_FROM_YAML)
{
}

void CommandLoadInnerEmitterFromYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    InnerEmitterParticleEditorNode* innerEmitterNode = dynamic_cast<InnerEmitterParticleEditorNode*>(selectedNode);
    if (!innerEmitterNode || !innerEmitterNode->GetInnerEmitter() ||!innerEmitterNode->GetParticleLayer())
    {
        return;
    }
    
    QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                    projectPath, QString("YAML File (*.yaml)"));
	if (filePath.isEmpty())
    {
		return;
    }

    ParticleEmitter* innerEmitter = innerEmitterNode->GetInnerEmitter();
    innerEmitter->LoadFromYaml(filePath.toStdString());

	// No additional NULL check is needed here - already performed at the beginning.
	QFileInfo fileInfo(filePath);
	ParticleLayer* innerEmitterLayer = innerEmitterNode->GetParticleLayer();
	innerEmitterLayer->innerEmitterPath = FilePath(fileInfo.path().toStdString(), fileInfo.fileName().toStdString());

	// Perform the validation of the Yaml file loaded.
	String validationMessage;
	if (ParticlesEditorSceneDataHelper::ValidateParticleEmitter(innerEmitter, validationMessage) == false)
	{
		ShowErrorDialog(validationMessage);
	}

	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandSaveInnerEmitterToYaml::CommandSaveInnerEmitterToYaml(bool forceAskFilename) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_SAVE_INNER_EMITTER_TO_YAML)
{
    this->forceAskFilename = forceAskFilename;
}

void CommandSaveInnerEmitterToYaml::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    InnerEmitterParticleEditorNode* emitterNode = dynamic_cast<InnerEmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode || !emitterNode->GetInnerEmitter())
    {
        return;
    }
	
    ParticleEmitter * emitter = emitterNode->GetInnerEmitter();
	FilePath yamlPath = emitter->GetConfigPath();
    if (this->forceAskFilename || yamlPath.IsEmpty() )
    {
        QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
		
        if (filePath.isEmpty())
        {
            return;
        }
        
        yamlPath = FilePath(filePath.toStdString());
    }

    emitter->SaveToYaml(yamlPath);
}
 */
