#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../ParticlesEditor/ParticlesEditorControl.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneGraph.h"

#include "DockParticleEditor/ParticlesEditorController.h"
#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"

#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/GUIState.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "SceneEditor/EditorSettings.h"
#include <QFileDialog>
#include <QString>

using namespace DAVA;

CommandOpenParticleEditorConfig::CommandOpenParticleEditorConfig()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandOpenParticleEditorConfig::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	ParticlesEditorControl * editor = screen->GetParticlesEditor();
	String currentPath = editor->GetActiveConfigName();
	if(currentPath.empty())
	{
		currentPath = editor->GetConfigsPath();
	}

	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle effect"), QString(currentPath.c_str()), QString("Effect File (*.yaml)"));

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		screen->GetParticlesEditor()->LoadFromYaml(selectedPathname);
		screen->FindCurrentBody()->bodyControl->GetSceneGraph()->UpdatePropertyPanel();
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

CommandSaveParticleEditorConfig::CommandSaveParticleEditorConfig()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandSaveParticleEditorConfig::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	ParticlesEditorControl * editor = screen->GetParticlesEditor();
	String currentPath = editor->GetActiveConfigName();
	if(currentPath.empty())
	{
		currentPath = editor->GetConfigsPath();
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save particle effect"), QString(currentPath.c_str()), QString("Effect File (*.yaml)"));
	if(filePath.size() > 0)
	{
		String normalizedPathname = PathnameToDAVAStyle(filePath);
		screen->GetParticlesEditor()->SaveToYaml(normalizedPathname);
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

CommandOpenParticleEditorSprite::CommandOpenParticleEditorSprite()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandOpenParticleEditorSprite::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	ParticlesEditorControl * editor = screen->GetParticlesEditor();
	editor->PackSprites();
	String currentPath = editor->GetActiveSpriteName();
	if(currentPath.empty())
	{
		currentPath = editor->GetSpritesDataPath();
	}
	else
	{
		currentPath = editor->GetActiveConfigFolder()+currentPath;
	}

	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open sprite"), QString(currentPath.c_str()), QString("Sprite (*.txt)"));

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		uint32 pos = selectedPathname.find(".txt");
		selectedPathname = selectedPathname.substr(0, pos);
		String relativePath = FileSystem::AbsoluteToRelativePath(editor->GetActiveConfigFolder(), selectedPathname);
		screen->GetParticlesEditor()->SetActiveSprite(relativePath);
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitter* emitter):
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->emitter = emitter;
}

void CommandUpdateEmitter::Init(ParticleEmitter::eType type,
								RefPtr<PropertyLine<float32> > emissionAngle,
								RefPtr<PropertyLine<float32> > emissionRange,
								RefPtr<PropertyLine<Vector3> > emissionVector,
								RefPtr<PropertyLine<float32> > radius,
								RefPtr<PropertyLine<Color> > colorOverLife,
								RefPtr<PropertyLine<Vector3> > size,
								float32 life)
{
	this->type = type;
	this->emissionAngle = emissionAngle;
	this->emissionRange = emissionRange;
	this->emissionVector = emissionVector;
	this->radius = radius;
	this->colorOverLife = colorOverLife;
	this->size = size;
	this->life = life;
}

void CommandUpdateEmitter::Execute()
{
	DVASSERT(emitter);

	emitter->type = type;
	emitter->emissionAngle = emissionAngle;
	emitter->emissionRange = emissionRange;
	emitter->emissionVector = emissionVector;
	emitter->radius = radius;
	emitter->colorOverLife = colorOverLife;
	emitter->size = size;
	emitter->SetLifeTime(life);
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleLayer* layer) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
}

void CommandUpdateParticleLayer::Init(bool isDisabled,
									  Sprite* sprite,
									  RefPtr< PropertyLine<float32> > life,
									  RefPtr< PropertyLine<float32> > lifeVariation,
									  RefPtr< PropertyLine<float32> > number,
									  RefPtr< PropertyLine<float32> > numberVariation,
									  RefPtr< PropertyLine<Vector2> > size,
									  RefPtr< PropertyLine<Vector2> > sizeVariation,
									  RefPtr< PropertyLine<float32> > sizeOverLife,
									  RefPtr< PropertyLine<float32> > velocity,
									  RefPtr< PropertyLine<float32> > velocityVariation,
									  RefPtr< PropertyLine<float32> > velocityOverLife,
									  RefPtr< PropertyLine<float32> > spin,
									  RefPtr< PropertyLine<float32> > spinVariation,
									  RefPtr< PropertyLine<float32> > spinOverLife,
									  RefPtr< PropertyLine<float32> > motionRandom,
									  RefPtr< PropertyLine<float32> > motionRandomVariation,
									  RefPtr< PropertyLine<float32> > motionRandomOverLife,
									  RefPtr< PropertyLine<float32> > bounce,
									  RefPtr< PropertyLine<float32> > bounceVariation,
									  RefPtr< PropertyLine<float32> > bounceOverLife,
									  RefPtr< PropertyLine<Color> > colorRandom,
									  RefPtr< PropertyLine<float32> > alphaOverLife,
									  RefPtr< PropertyLine<Color> > colorOverLife,
									  RefPtr< PropertyLine<float32> > frameOverLife,
									  float32 alignToMotion,
									  float32 startTime,
									  float32 endTime)
{
	this->isDisabled = isDisabled;
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
	this->motionRandom = motionRandom;
	this->motionRandomVariation = motionRandomVariation;
	this->motionRandomOverLife = motionRandomOverLife;
	this->bounce = bounce;
	this->bounceVariation = bounceVariation;
	this->bounceOverLife = bounceOverLife;
	this->colorRandom = colorRandom;
	this->alphaOverLife = alphaOverLife;
	this->colorOverLife = colorOverLife;
	this->alignToMotion = alignToMotion;
	this->frameOverLife = frameOverLife;
	this->startTime = startTime;
	this->endTime = endTime;
}


void CommandUpdateParticleLayer::Execute()
{
	layer->isDisabled = isDisabled;
	if (layer->GetSprite() != sprite)
	{
		layer->SetSprite(sprite);
	}
	layer->life = life;
	layer->lifeVariation = lifeVariation;
	layer->number = number;
	layer->numberVariation = numberVariation;
	layer->size = size;
	layer->sizeVariation = sizeVariation;
	layer->sizeOverLife = sizeOverLife;
	layer->velocity = velocity;
	layer->velocityVariation = velocityVariation;
	layer->velocityOverLife = velocityOverLife;
	layer->spin = spin;
	layer->spinVariation = spinVariation;
	layer->spinOverLife = spinOverLife;
	layer->motionRandom = motionRandom;
	layer->motionRandomVariation = motionRandomVariation;
	layer->motionRandomOverLife = motionRandomOverLife;
	layer->bounce = bounce;
	layer->bounceVariation = bounceVariation;
	layer->bounceOverLife = bounceOverLife;
	layer->colorRandom = colorRandom;
	layer->alphaOverLife = alphaOverLife;
	layer->colorOverLife = colorOverLife;
	layer->frameOverLife = frameOverLife;
	layer->alignToMotion = alignToMotion;
	layer->startTime = startTime;
	layer->endTime = endTime;
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
	this->startTime = startTime;
	this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Execute()
{
	layer->startTime = startTime;
	layer->endTime = endTime;
}

CommandUpdateParticleLayerForce::CommandUpdateParticleLayerForce(ParticleLayer* layer, uint32 forceId) :
	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
	this->layer = layer;
	this->forceId = forceId;
}

void CommandUpdateParticleLayerForce::Init(RefPtr< PropertyLine<Vector3> > force,
										   RefPtr< PropertyLine<Vector3> > forcesVariation,
										   RefPtr< PropertyLine<float32> > forcesOverLife)
{
	this->force = force;
	this->forcesVariation = forcesVariation;
	this->forcesOverLife = forcesOverLife;
}

void CommandUpdateParticleLayerForce::Execute()
{
	layer->forces[forceId] = force;
	layer->forcesVariation[forceId] = forcesVariation;
	layer->forcesOverLife[forceId] = forcesOverLife;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

CommandAddParticleEmitter::CommandAddParticleEmitter() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitter::Execute()
{
    // This command is done through Main Window to reuse the existing code.
    QtMainWindowHandler::Instance()->CreateParticleEmitterNode();
}


CommandStartStopParticleEffect::CommandStartStopParticleEffect(bool isStart) :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->isStart = isStart;
}

void CommandStartStopParticleEffect::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EffectParticleEditorNode* effectNode = dynamic_cast<EffectParticleEditorNode*>(selectedNode);
    if (!effectNode || !effectNode->GetRootNode())
    {
        return;
    }
    
    if (this->isStart)
    {
        effectNode->GetRootNode()->Start();
    }
    else
    {
        effectNode->GetRootNode()->Stop();
    }
}

CommandRestartParticleEffect::CommandRestartParticleEffect() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRestartParticleEffect::Execute()
{
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EffectParticleEditorNode* effectNode = dynamic_cast<EffectParticleEditorNode*>(selectedNode);
    if (!effectNode || !effectNode->GetRootNode() || !effectNode->GetRootNode())
    {
        return;
    }

    effectNode->GetRootNode()->Restart();
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitterLayer::Execute()
{
    // Need to know selected Particle Emitter Layers Root.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    EmitterParticleEditorNode* emitterNode = dynamic_cast<EmitterParticleEditorNode*>(selectedNode);
    if (!emitterNode)
    {
        return;
    }

	emitterNode->GetRootNode()->Stop();
	
    // Lets select this node when the tree will be rebuilt.
    LayerParticleEditorNode* layerNode = ParticlesEditorController::Instance()->AddParticleLayerToNode(emitterNode);
    if (layerNode)
    {
        layerNode->SetMarkedToSelection(true);
    }

	emitterNode->GetRootNode()->Restart();

    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveParticleEmitterLayer::Execute()
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

	ParticleEffectNode* rootNode = layerNode->GetRootNode();
	if (rootNode)
		rootNode->Stop();
    ParticlesEditorController::Instance()->RemoveParticleLayerNode(layerNode);
	if (rootNode)
		rootNode->Restart();
	
    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandCloneParticleEmitterLayer::Execute()
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
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandAddParticleEmitterForce::CommandAddParticleEmitterForce() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandAddParticleEmitterForce::Execute()
{
    // Need to know selected Layer Node to add the Force to.
    BaseParticleEditorNode* selectedNode = ParticlesEditorController::Instance()->GetSelectedNode();
    LayerParticleEditorNode* layerNode = dynamic_cast<LayerParticleEditorNode*>(selectedNode);
    if (!layerNode || !layerNode->GetRootNode())
    {
        return;
    }

	layerNode->GetRootNode()->Stop();
    ForceParticleEditorNode* forceNode = ParticlesEditorController::Instance()->AddParticleForceToNode(layerNode);
    if (forceNode)
    {
        forceNode->SetMarkedToSelection(true);
    }
	layerNode->GetRootNode()->Restart();
	
    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandRemoveParticleEmitterForce::CommandRemoveParticleEmitterForce() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveParticleEmitterForce::Execute()
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

	ParticleEffectNode* rootNode = forceNode->GetRootNode();
	if (rootNode)
		rootNode->Stop();
    ParticlesEditorController::Instance()->RemoveParticleForceNode(forceNode);
	if (rootNode)
		rootNode->Restart();

    // Update the scene graph.
    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml() :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
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
    
    QString projectPath = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
                                                    projectPath, QString("YAML File (*.yaml)"));
	if (filePath.isEmpty())
    {
		return;
    }

    // In case this emitter already has Editor Nodes - remove them before loading.
    ParticlesEditorController::Instance()->CleanupParticleEmitterEditorNode(emitterNode);
    emitterNode->GetEmitterNode()->LoadFromYaml(filePath.toStdString());

    QtMainWindowHandler::Instance()->RefreshSceneGraph();
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(bool forceAskFilename) :
    Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
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

    String yamlPath = emitterNode->GetEmitterNode()->GetYamlPath();

    if (this->forceAskFilename || yamlPath.empty() )
    {
        QString projectPath = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
 
        if (filePath.isEmpty())
        {
            return;
        }
        
        yamlPath = filePath.toStdString();
    }

    emitterNode->GetEmitterNode()->SaveToYaml(yamlPath);
}

