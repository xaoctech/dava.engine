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
    pendingComponents.insert(prefabComponent);
}

void PrefabLoadingSystem::RemoveComponent(Entity* entity, Component* component)
{
    PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);

    pendingComponents.erase(prefabComponent);

    PrefabSingleComponent* prefabSingleComponent = GetScene()->GetSingletonComponent<PrefabSingleComponent>();
    prefabSingleComponent->changedPrefabComponent.erase(prefabComponent);

    auto iter = loadedPrefabs.find(prefabComponent);
    if (iter != loadedPrefabs.end())
    {
        const Vector<Entity*>& prefabEntities = iter->second;

        for (Entity* e : prefabEntities)
        {
            entity->RemoveNode(e);
            SafeRelease(e);
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
    PrefabSingleComponent* prefabSingleComponent = GetScene()->GetSingletonComponent<PrefabSingleComponent>();
    prefabSingleComponent->changedPrefabComponent.clear();
}

void PrefabLoadingSystem::Process(float32 delta)
{
    for (PrefabComponent* component : pendingComponents)
    {
        Entity* parentEntity = component->GetEntity();
        Asset<Prefab> prefab = component->GetPrefab();
        if (prefab != nullptr)
        {
            DVASSERT(loadedPrefabs.count(component) == 0);
            Vector<Entity*>& prefabEntities = loadedPrefabs[component];
            prefabEntities = prefab->ClonePrefabEntities();
            for (Entity* e : prefabEntities)
            {
                e->AddComponent(new RuntimeEntityMarkComponent());
                e->SetVisible(parentEntity->GetVisible());
                parentEntity->AddNode(e);
            }
        }
    }
    pendingComponents.clear();

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
