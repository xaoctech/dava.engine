#include "Classes/LandscapeEditor/Private/MassObjectCreationSystem.h"
#include "Classes/LandscapeEditor/Private/MassObjectCreationComponents.h"

MassObjectCreationSystem::MassObjectCreationSystem(DAVA::Scene* scene)
    : SceneSystem(scene, 0)
{
    layers.values.push_back(std::pair<DAVA::Entity*, DAVA::String>(scene, "Scene"));
    layersToCreatedObjects[scene] = DAVA::Set<DAVA::Entity*>();
    currentLayer = scene;
}

void MassObjectCreationSystem::RegisterEntity(DAVA::Entity* entity)
{
    MassObjectCreationLayer* layerComponent = entity->GetComponent<MassObjectCreationLayer>();
    MassCreatedObjectComponent* createdObjectComponent = entity->GetComponent<MassCreatedObjectComponent>();

    DVASSERT(layerComponent == nullptr || createdObjectComponent == nullptr);

    if (layerComponent != nullptr)
    {
        layers.values.push_back(std::pair<DAVA::Entity*, DAVA::String>(entity, layerComponent->GetName().c_str()));
        DVASSERT(layersToCreatedObjects.count(entity) == 0);
        layersToCreatedObjects[entity] = DAVA::Set<DAVA::Entity*>();
    }

    if (createdObjectComponent != nullptr)
    {
        DAVA::Entity* layer = entity->GetParent();
        DVASSERT(layer == GetScene() || layer->GetComponent<MassObjectCreationLayer>());
        layersToCreatedObjects[layer].insert(entity);
    }
}

void MassObjectCreationSystem::UnregisterEntity(DAVA::Entity* entity)
{
    if (currentLayer == entity)
    {
        currentLayer = GetScene();
    }

    MassObjectCreationLayer* layerComponent = entity->GetComponent<MassObjectCreationLayer>();
    MassCreatedObjectComponent* createdObjectComponent = entity->GetComponent<MassCreatedObjectComponent>();

    DVASSERT(layerComponent == nullptr || createdObjectComponent == nullptr);

    if (createdObjectComponent != nullptr)
    {
        DAVA::Entity* layer = entity->GetParent();
        DVASSERT(layer != nullptr);
        DVASSERT(layer == GetScene() || layer->GetComponent<MassObjectCreationLayer>());
        layersToCreatedObjects[layer].erase(entity);
    }

    if (layerComponent != nullptr)
    {
        auto iter = std::remove_if(layers.values.begin(), layers.values.end(), [entity](const std::pair<DAVA::Entity*, DAVA::String>& p) {
            return p.first == entity;
        });

        layers.values.erase(iter, layers.values.end());
        layersToCreatedObjects.erase(entity);
    }
}

void MassObjectCreationSystem::PrepareForRemove()
{
    currentLayer = nullptr;
    layers.values.clear();
    layersToCreatedObjects.clear();
}

const DAVA::ReflectedPairsVector<DAVA::Entity*, DAVA::String>& MassObjectCreationSystem::GetLayers() const
{
    return layers;
}

DAVA::Entity* MassObjectCreationSystem::GetEditedLayer() const
{
    return currentLayer;
}

void MassObjectCreationSystem::SetEditedLayer(DAVA::Entity* layer)
{
    currentLayer = layer;
}

const DAVA::Set<DAVA::Entity*>& MassObjectCreationSystem::GetEntitiesInLayer(DAVA::Entity* layer) const
{
    DVASSERT(layer == GetScene() || layer->GetComponent<MassObjectCreationLayer>() != nullptr);

    auto iter = layersToCreatedObjects.find(layer);
    DVASSERT(iter != layersToCreatedObjects.end());

    return iter->second;
}
