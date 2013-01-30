//
//  ParticlesEditorSceneDataHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/25/12.
//
//

#include "ParticlesEditorSceneDataHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

bool ParticlesEditorSceneDataHelper::AddSceneNode(SceneNode* node) const
{
	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(node->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    if (emitterComponent)
    {
        ParticlesEditorController::Instance()->AddParticleEmitterNodeToScene(node);
        return true;
    }
    
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent == NULL)
    {
        // Not relevant.
        return false;
    }
	
    // Register in Particles Editor.
    ParticlesEditorController::Instance()->RegisterParticleEffectNode(node);
    return false;
}

void ParticlesEditorSceneDataHelper::RemoveSceneNode(SceneNode *node) const
{
    // If the Particle Effect node is removed - unregister it.
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent)
    {
        ParticlesEditorController::Instance()->UnregiserParticleEffectNode(node);
    }

    // If the Particle Emitter node is removed - remove it from the tree.
    ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(node->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    if (emitterComponent)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(node);
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
