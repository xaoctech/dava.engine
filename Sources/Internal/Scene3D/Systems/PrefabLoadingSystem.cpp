#include "Scene3D/Systems/PrefabLoadingSystem.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Components/RuntimeEntityMarkComponent.h"
#include "Scene3D/Components/SingleComponents/PrefabSingleComponent.h"
#include "Scene3D/Scene.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
PrefabLoadingSystem::PrefabLoadingSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<PrefabComponent>())
{
}

PrefabLoadingSystem::~PrefabLoadingSystem()
{
    DVASSERT(loadedPrefabs.empty());
}

void PrefabLoadingSystem::AddEntity(Entity* entity)
{
    for (uint32 i = 0; i < entity->GetComponentCount<PrefabComponent>(); ++i)
    {
        AddComponent(entity, entity->GetComponent<PrefabComponent>(i));
    }
}

void PrefabLoadingSystem::RemoveEntity(Entity* entity)
{
    for (uint32 i = 0; i < entity->GetComponentCount<PrefabComponent>(); ++i)
    {
        RemoveComponent(entity, entity->GetComponent<PrefabComponent>(i));
    }
}

void PrefabLoadingSystem::AddComponent(Entity* entity, Component* component)
{
    PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);
    Asset<Prefab> prefab = prefabComponent->GetPrefab();
    if (prefab != nullptr)
    {
        DVASSERT(loadedPrefabs.count(prefabComponent) == 0);
        Vector<Entity*>& prefabEntities = loadedPrefabs[prefabComponent];
        prefabEntities = prefab->ClonePrefabEntities();
        for (Entity* e : prefabEntities)
        {
            e->AddComponent(new RuntimeEntityMarkComponent());
            entity->AddNode(e);
        }
    }
}

void PrefabLoadingSystem::RemoveComponent(Entity* entity, Component* component)
{
    PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);

    auto iter = loadedPrefabs.find(prefabComponent);
    if (iter != loadedPrefabs.end())
    {
        const Vector<Entity*>& prefabEntities = iter->second;

        for (Entity* entity : prefabEntities)
        {
            entity->RemoveNode(entity);
            SafeRelease(entity);
        }

        loadedPrefabs.erase(iter);
    }
}

void PrefabLoadingSystem::PrepareForRemove()
{
    for (const auto& node : loadedPrefabs)
    {
        Entity* parentEntity = node.first->GetEntity();
        for (Entity* e : node.second)
        {
            parentEntity->RemoveNode(e);
            SafeRelease(e);
        }
    }

    loadedPrefabs.clear();
}

void PrefabLoadingSystem::Process(float32 delta)
{
    PrefabSingleComponent* prefabSingleComponent = GetScene()->GetSingletonComponent<PrefabSingleComponent>();

    for (PrefabComponent* prefabComponent : prefabSingleComponent->changedPrefabComponent)
    {
        RemoveComponent(prefabComponent->GetEntity(), prefabComponent);
        AddComponent(prefabComponent->GetEntity(), prefabComponent);
    }

    prefabSingleComponent->changedPrefabComponent.clear();
}

DAVA_VIRTUAL_REFLECTION_IMPL(PrefabLoadingSystem)
{
    ReflectionRegistrator<PrefabLoadingSystem>::Begin()
    .End();
}

} // namespace DAVA
