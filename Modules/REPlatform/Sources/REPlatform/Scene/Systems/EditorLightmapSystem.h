#pragma once

#include <Base/BaseTypes.h>
#include <Base/UnordererMap.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class LightmapComponent;
class EditorLightmapSystem : public SceneSystem
{
public:
    EditorLightmapSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void Process(float32 delta) override;

protected:
    void UpdateDynamicParams(LightmapComponent* component);
    void RemoveDynamicParams(LightmapComponent* component);
};
} // namespace DAVA
