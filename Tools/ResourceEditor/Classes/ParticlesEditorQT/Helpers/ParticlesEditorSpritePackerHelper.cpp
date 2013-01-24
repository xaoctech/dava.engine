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
	List<ParticleEffectNode*> particleEffects;
	sceneData->GetAllParticleEffects(particleEffects);

	for(auto it = particleEffects.begin(); it != particleEffects.end(); ++it)
	{
		ParticleEffectNode* curNode = (*it);
		curNode->Stop();

		// All the children of the Particle Effect Node are currently Emitters.
		int32 emittersCount = curNode->GetChildrenCount();
		for (int32 i = 0; i < emittersCount; i ++)
		{
			ParticleEmitterNode* emitterNode = dynamic_cast<ParticleEmitterNode*>(curNode->GetChild(i));
			if (!emitterNode || !emitterNode->GetEmitter())
			{
				continue;
			}
			
			emitterNode->GetEmitter()->ReloadLayerSprites();
		}

		curNode->Start();
	}
}
