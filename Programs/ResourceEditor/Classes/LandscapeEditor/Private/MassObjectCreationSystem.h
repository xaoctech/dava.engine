#pragma once

#include <REPlatform/Scene/Systems/EditorSceneSystem.h>

#include <TArc/Utils/ReflectedPairsVector.h>

#include <Entity/SceneSystem.h>
#include <Scene3D/Scene.h>

class MassObjectCreationLayer;
class MassCreatedObjectComponent;

class MassObjectCreationSystem : public DAVA::SceneSystem, public DAVA::EditorSceneSystem
{
public:
    MassObjectCreationSystem(DAVA::Scene* scene);

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;

    void PrepareForRemove() override;

    const DAVA::ReflectedPairsVector<DAVA::Entity*, DAVA::String>& GetLayers() const;
    DAVA::Entity* GetEditedLayer() const;
    void SetEditedLayer(DAVA::Entity* layer);

    const DAVA::Set<DAVA::Entity*>& GetEntitiesInLayer(DAVA::Entity* layer) const;

private:
    DAVA::Entity* currentLayer = nullptr;
    DAVA::ReflectedPairsVector<DAVA::Entity*, DAVA::String> layers;

    DAVA::Map<DAVA::Entity*, DAVA::Set<DAVA::Entity*>> layersToCreatedObjects;
};