#include "LevelStreamingSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Level.h"

#include "Engine/Engine.h"

namespace DAVA
{
LevelStreamingSystem::LevelStreamingSystem(Scene* scene)
    : SceneSystem(scene)
{
}

LevelStreamingSystem::~LevelStreamingSystem()
{
}

void LevelStreamingSystem::LoadLevel(const FilePath& filepath)
{
    level = GetEngineContext()->assetManager->LoadAsset<Level>(filepath, MakeFunction(this, &LevelStreamingSystem::LoadLevelCompleteCallback));
    status = LEVEL_BASE_PART_LOADING;
}

void LevelStreamingSystem::LoadLevelCompleteCallback(Asset<AssetBase> asset)
{
    status = LEVEL_STREAMING;
    level = std::dynamic_pointer_cast<Level>(asset);

    /*
     level->AddLoadedEntitiesToScene(GetScene());
    */

    level->StartStreaming(MakeFunction(this, &LevelStreamingSystem::StreamingCallback));
}

void LevelStreamingSystem::StreamingCallback()
{
    const Vector<Level::StreamingEvent>& streamingEvents = level->GetActiveStreamingEvents();

    /*size_t eventCount = streamingEvents.size();
    for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
    {
        const Level::StreamingEvent& event = streamingEvents[eventIndex];
        if (event.type == Level::StreamingEvent::ENTITY_ADDED)
        {
            GetScene()->AddNode(event.entity);
        }else if (event.type == Level::StreamingEvent::ENTITY_REMOVED)
        {
            GetScene()->DeleteNode(event.entity);
        }
    }
    level->ClearActiveStreamingEvents();*/
}

void LevelStreamingSystem::UnloadLevel()
{
    level->StopStreaming();

    const Vector<Entity*>& activeEntities = level->GetActiveEntities();
    size_t entitiesCount = activeEntities.size();
    for (size_t k = 0; k < entitiesCount; ++k)
    {
        //GetScene()->DeleteNode(activeEntities[k]);
    }

    level = nullptr;
}

void LevelStreamingSystem::Process(float32 timeElapsed)
{
    //DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_WAVE_SYSTEM);
    ///CameraSingleComponent* cameraSingleComponent = scene->GetSingleComponent<CameraSingleComponent>();
    //Camera* activeCamera = cameraSingleComponent->activeCamera;
    //level->UpdateCamera(activeCamera);
}
};
