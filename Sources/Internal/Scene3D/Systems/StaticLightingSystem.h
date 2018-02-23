#pragma once

#include "Base/BaseTypes.h"
#include "Base/UnordererSet.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Scene;
class Entity;
class Texture;
class RenderBatch;
class LightmapComponent;
class LightmapDataComponent;
class LightmapSingleComponent;
class StaticLightingSystem : public SceneSystem
{
public:
    StaticLightingSystem(Scene* scene);
    ~StaticLightingSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    void InvalidateLightmapData();
    void UpdateLightmapData();

    LightmapDataComponent* FindLightmapData(BaseObject* object);
    LightmapDataComponent* FindLightmapData(Entity* entity);

protected:
    void UpdateDynamicParams(LightmapDataComponent* component);
    void UpdateDynamicParams(BaseObject* object, const void* uvParam, rhi::HTexture texture);

    void RemoveDynamicParams(LightmapDataComponent* component);
    void RemoveDynamicParams(BaseObject* object);

    void SetDefaultLighmap(LightmapComponent* component);

    Vector<LightmapComponent*> lightmapComponents;
    Vector<LightmapDataComponent*> lightmapDataComponents;

    rhi::HTexture defaultLightmapTexture;
    Vector4 defaultLightmapUV = Vector4(0.f, 0.f, 1.f, 1.f);

    LightmapSingleComponent* lightmapSingleComponent = nullptr;

    DAVA_VIRTUAL_REFLECTION(StaticLightingSystem, SceneSystem);
};
};
