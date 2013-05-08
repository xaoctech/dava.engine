//
//  ParticlesEditorSceneDataHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/25/12.
//
//

#include "ParticlesEditorSceneDataHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

bool ParticlesEditorSceneDataHelper::AddSceneNode(Entity* node) const
{
	ParticleEmitter * emitter = GetEmitter(node);
    if (emitter)
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

void ParticlesEditorSceneDataHelper::RemoveSceneNode(Entity *node) const
{
    // If the Particle Effect node is removed - unregister it.
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent)
    {
        ParticlesEditorController::Instance()->UnregiserParticleEffectNode(node);
    }

    // If the Particle Emitter node is removed - remove it from the tree.
    ParticleEmitter * emitter = GetEmitter(node);
    if (emitter)
    {
        ParticlesEditorController::Instance()->RemoveParticleEmitterNode(node);
    }
}

bool ParticlesEditorSceneDataHelper::ValidateParticleEmitter(ParticleEmitter * emitter, String& validationMsg)
{
	if (!emitter)
	{
		return true;
	}
	
	if (emitter->Is3DFlagCorrect())
	{
		return true;
	}

	// Don't use Format() helper here - the string with path might be too long for Format().
	validationMsg = ("\"3d\" flag value is wrong for Particle Emitter Configuration file ");
	validationMsg += emitter->GetConfigPath().GetAbsolutePathname().c_str();
	validationMsg += ". Please verify whether you are using the correct configuration file.\n\"3d\" flag for this Particle Emitter will be reset to TRUE.";
	
	// Yuri Coder, 2013/05/08. Since Particle Editor works with 3D Particles only - have to set this flag
	// manually.
	emitter->Set3D(true);

	return false;
}
