#pragma once

#include "Entity/SceneSystem.h"
#include "Scene3D/Components/LightRenderComponent.h"

namespace DAVA
{
class LightComponent;
class LightRenderSystem : public SceneSystem
{
public:
    LightRenderSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void ImmediateEvent(Component* component, uint32 event) override;

    void InvalidateMaterials();

private:
    using LightComponentsPair = std::pair<LightComponent*, LightRenderComponent*>;
    Vector<LightComponentsPair> allComponents;
    RenderSystem* renderSystem = nullptr;
};
};
