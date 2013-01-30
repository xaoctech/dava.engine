//
//  ParticlesEditorSceneDataHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/25/12.
//
//

#include "ParticlesEditorSceneDataHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"
using namespace DAVA;

bool ParticlesEditorSceneDataHelper::AddSceneNode(SceneNode* node) const
{
    ParticleEmitterNode* emitterSceneNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (emitterSceneNode)
    {
        ParticlesEditorController::Instance()->AddParticleEmitterNodeToScene(emitterSceneNode);
        return true;
    }
    
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode == NULL)
    {
        // Not relevant.
        return false;
    }
	
    // Register in Particles Editor.
    ParticlesEditorController::Instance()->RegisterParticleEffectNode(effectNode);
    return false;
}

void ParticlesEditorSceneDataHelper::RemoveSceneNode(SceneNode *node) const
{
    // If the Particle Effect node is removed - unregister it.
    ParticleEffectNode* effectNode = dynamic_cast<ParticleEffectNode*>(node);
    if (effectNode)
    {
        ParticlesEditorController::Instance()->UnregiserParticleEffectNode(effectNode);
    }
    
    // If the Particle Emitter node is removed - remove it from the tree.
    ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (emitterNode)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(emitterNode);
    }
}

 bool ParticlesEditorSceneDataHelper::ValidateParticleEmitterNode(ParticleEmitterNode* node, String& validationMsg)
{
	if (!node || !node->GetEmitter())
	{
		return true;
	}
	
	if (node->GetEmitter()->Is3DFlagCorrect())
	{
		return true;
	}
	
	validationMsg = Format("\"3d\" flag value is wrong for Particle Emitter Configuration file %s. Please verify whether you are using the correct configuration file.",
						   node->GetEmitter()->GetConfigPath().c_str());
	return false;
}