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
    struct DecalBuildInfo
    {
        RenderObject* object = nullptr;
        RenderBatch* batch = nullptr;
        AABBox3 box;
        int32 lodIndex = -1;
        int32 switchIndex = -1;
        Vector3 projectionAxis;
        Matrix4 projectionSpaceTransform;
        Matrix4 projectionSpaceInverseTransform;
    };
    void BuildDecal(Entity* entity, GeoDecalComponent* component);
    void RemoveCreatedDecals(Entity* entity, GeoDecalComponent* component);
    void BuildDecal(const DecalBuildInfo& info);

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
