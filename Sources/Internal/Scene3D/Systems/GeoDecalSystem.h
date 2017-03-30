#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Components/GeoDecalComponent.h"

namespace DAVA
{
class GeoDecalSystem : public SceneSystem
{
public:
    GeoDecalSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;

private:
    void BuildDecal(Entity* entity, GeoDecalComponent* component);
    void RemoveCreatedDecals(GeoDecalComponent* component);

private:
    struct GeoDecalCache
    {
        GeoDecalCache() = default;
        GeoDecalCache(Entity* entity);
        Map<Component*, GeoDecalComponent::Config> lastValidConfig;
    };
    Map<Entity*, GeoDecalCache> decals;
};
}
