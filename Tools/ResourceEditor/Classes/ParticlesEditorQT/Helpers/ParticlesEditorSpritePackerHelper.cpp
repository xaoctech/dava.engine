#include "ParticlesEditorSpritePackerHelper.h"
#include "../SpritesPacker.h"
#include "../SceneEditor/EditorSettings.h"
#include "./Scene/SceneDataManager.h"

using namespace DAVA;

void ParticlesEditorSpritePackerHelper::UpdateParticleSprites()
{
	SpritesPacker packer;
	packer.SetInputDir(EditorSettings::Instance()->GetProjectPath()+"DataSource/Gfx/Particles");
	packer.SetOutputDir(EditorSettings::Instance()->GetProjectPath()+"Data/Gfx/Particles");
	packer.Pack();

	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		if(NULL != sceneData)
		{
			UpdateParticleSprites(sceneData);
		}
	}
}

void ParticlesEditorSpritePackerHelper::UpdateParticleSprites(SceneData* sceneData)
{
	// recursive walkthrough
	// for each layer found - call Reload().
	List<Sprite*> sprites;
	sceneData->GetAllSprites(sprites);
	for(auto it = sprites.begin(); it != sprites.end(); ++it)
	{
		(*it)->Reload();
	}
}
