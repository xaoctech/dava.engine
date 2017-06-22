#pragma once

#include <Entity/SceneSystem.h>

namespace DAVA
{
class RenderObject;
class PhysicsDebugDrawSystem : public SceneSystem
{
public:
    PhysicsDebugDrawSystem();

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

private:
    UnorderedMap<Component*, RenderObject*> renderObjects;
};
} // namespace DAVA