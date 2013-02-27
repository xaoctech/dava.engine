#include "SpritePackerHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"

#include "../SpritesPacker.h"
#include "../SceneEditor/EditorSettings.h"
#include "./Scene/SceneDataManager.h"

#include <QtConcurrentRun>

#include "ResourcePackerScreen.h"
#include "Platform/Qt/QtLayer.h"

using namespace DAVA;

SpritePackerHelper::SpritePackerHelper()
{
	QObject::connect(&watcher, SIGNAL(finished()), this, SLOT(threadRepackAllFinished()), Qt::QueuedConnection);
}

void SpritePackerHelper::UpdateParticleSprites()
{
	String projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.empty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	Pack();
	
	Reload();
}

void SpritePackerHelper::Pack()
{
	void *pool = DAVA::QtLayer::Instance()->CreateAutoreleasePool();
	String projectPath = EditorSettings::Instance()->GetProjectPath();
	String inputDir = projectPath+"DataSource/Gfx/Particles";
	String outputDir = projectPath+"Data/Gfx/Particles";

	if(!FileSystem::Instance()->IsDirectory(inputDir))
	{
		return;
	}

	ResourcePackerScreen * resourcePackerScreen = new ResourcePackerScreen();
	
	bool isChanged = resourcePackerScreen->IsMD5ChangedDir(projectPath+"DataSource/Gfx",inputDir,"particles.md5",true);
	
	SafeRelease(resourcePackerScreen);
	if(!isChanged)
	{
		return;
	}
	
	SpritesPacker packer;
	packer.SetInputDir(inputDir);
	packer.SetOutputDir(outputDir);
	packer.Pack();
	DAVA::QtLayer::Instance()->ReleaseAutoreleasePool(pool);
}

void SpritePackerHelper::Reload()
{
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

void SpritePackerHelper::ReloadParticleSprites(SceneData* sceneData)
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
			ParticleEmitter * emitter = GetEmitter(childNode);

			if (!emitter)
			{
				continue;
			}
			
			emitter->ReloadLayerSprites();
		}

		effectComponent->Start();
	}
}

void SpritePackerHelper::UpdateParticleSpritesAsync()
{
	String projectPath = EditorSettings::Instance()->GetProjectPath();
    if(projectPath.empty())
    {
        Logger::Warning("[ParticlesEditorSpritePackerHelper::UpdateParticleSprites] Project path not set.");
        return;
    }

	if(NULL == future)
	{
		future = new QFuture<void>;
		*future = QtConcurrent::run(this, &SpritePackerHelper::Pack);
		watcher.setFuture(*future);
	}
}

void SpritePackerHelper::threadRepackAllFinished()
{
	future = NULL;
	
	Reload();

	emit readyAll();
}
