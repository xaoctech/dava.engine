#pragma once

#include "Entity/SceneSystem.h"

namespace DAVA
{
class GeoDecalRenderSystem : public SceneSystem
{
public:
    GeoDecalRenderSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
};
}
