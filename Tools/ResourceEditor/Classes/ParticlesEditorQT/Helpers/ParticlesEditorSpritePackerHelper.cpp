#include "ParticlesEditorSpritePackerHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"

#include "../SpritesPacker.h"
#include "../SceneEditor/EditorSettings.h"
#include "./Scene/SceneDataManager.h"

using namespace DAVA;

void ParticlesEditorSpritePackerHelper::UpdateParticleSprites()
{
    String projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.empty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }
    
	SpritesPacker packer;
	packer.SetInputDir(projectPath+"DataSource/Gfx/Particles");
	packer.SetOutputDir(projectPath+"Data/Gfx/Particles");
	packer.Pack();

	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		if(NULL != sceneData)
		{
			// All the Particle Effects must be re-started after sprites are reloaded to avoid
			// issue like DF-545.
			ReloadParticleSprites(sceneData);
		}
	}

	ParticlesEditorController::Instance()->RefreshSelectedNode();
}

void ParticlesEditorSpritePackerHelper::ReloadParticleSprites(SceneData* sceneData)
{
	List<SceneNode*> particleEffects;
	sceneData->GetAllParticleEffects(particleEffects);

	for (auto it = particleEffects.begin(); it != particleEffects.end(); ++it)
	{
		SceneNode* curNode = (*it);
	    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(curNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
		
		if (!effectComponent)
		{
			continue;
		}

		effectComponent->Stop();

		// All the children of this Scene Node must have Emitter components.
		int32 emittersCount = curNode->GetChildrenCount();
		for (int32 i = 0; i < emittersCount; i ++)
		{
			SceneNode* childNode = curNode->GetChild(i);
			ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(childNode->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));

			if (!emitterComponent || !emitterComponent->GetParticleEmitter())
			{
				continue;
			}
			
			emitterComponent->GetParticleEmitter()->ReloadLayerSprites();
		}

		effectComponent->Start();
	}
}
