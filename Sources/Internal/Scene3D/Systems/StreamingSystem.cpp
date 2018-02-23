#include "Engine/Engine.h"
#include "Scene3D/Systems/StreamingSystem.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Level.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
StreamingSystem::StreamingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
}

StreamingSystem::~StreamingSystem()
{
}

void StreamingSystem::RegisterEntity(Entity* entity)
{
    for (Component* comp : entity->GetComponents())
    {
        RegisterComponent(entity, comp);
    }
}
void StreamingSystem::UnregisterEntity(Entity* entity)
{
    for (Component* comp : entity->GetComponents())
    {
        UnregisterComponent(entity, comp);
    }
}

void StreamingSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Type::Instance<PrefabComponent>())
    {
        PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);
        if (prefabComponent->GetPrefab())
        {
            Vector<Entity*> prefabEntities = prefabComponent->GetPrefab()->GetPrefabEntities();
            for (Entity* prefabChildren : prefabEntities)
            {
                ScopedPtr<Entity> entityClone(prefabChildren->Clone());
                entity->AddEntity(entityClone);
            }
        }
    }
}

void StreamingSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Type::Instance<PrefabComponent>())
    {
        /* I am not sure that this is valid behavior. We can potentially delete more than added. */
        PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);
        entity->RemoveAllChildren();
    }
}

void StreamingSystem::PrepareForRemove()
{
}

void StreamingSystem::LoadLevel(const FilePath& filepath)
{
    level = GetEngineContext()->assetManager->LoadAsset<Level>(filepath, MakeFunction(this, &StreamingSystem::LoadLevelCompleteCallback));
}

void StreamingSystem::LoadLevelCompleteCallback(Asset<AssetBase> asset)
{
    status = LEVEL_STREAMING;
    level = std::dynamic_pointer_cast<Level>(asset);
    level->StartStreaming(MakeFunction(this, &StreamingSystem::StreamingCallback));
}

void StreamingSystem::StreamingCallback()
{
    const Vector<Level::StreamingEvent>& streamingEvents = level->GetActiveStreamingEvents();

    size_t eventCount = streamingEvents.size();
    for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
    {
        const Level::StreamingEvent& event = streamingEvents[eventIndex];
        if (event.type == Level::StreamingEvent::ENTITY_ADDED)
        {
            GetScene()->AddEntity(event.entity);
        }else if (event.type == Level::StreamingEvent::ENTITY_REMOVED)
        {
            GetScene()->RemoveEntity(event.entity);
        }
    }
    level->ClearActiveStreamingEvents();
}

void StreamingSystem::UnloadLevel()
{
    //    level->StopStreaming();
    //
    //    const Vector<Entity*>& activeEntities = level->GetActiveEntities();
    //    size_t entitiesCount = activeEntities.size();
    //    for (size_t k = 0; k < entitiesCount; ++k)
    //    {
    //        //GetScene()->DeleteNode(activeEntities[k]);
    //    }
    //
    //    level = nullptr;
}

void StreamingSystem::Process(float32 timeElapsed)
{
    if (level)
    {
        level->ProcessStreaming(GetScene()->GetCurrentCamera());
    }
}
};
