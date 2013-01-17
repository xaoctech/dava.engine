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

using namespace DAVA;

bool ParticlesEditorSceneDataHelper::AddSceneNode(SceneNode* node) const
{
	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(node->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    if (emitterComponent)
    {
        ParticlesEditorController::Instance()->AddParticleEmitterNodeToScene(node);
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
    ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(node->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    if (emitterComponent)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(node);
    }
}
