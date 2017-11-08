#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Component;
class BaseProcessSystem : public SceneSystem
{
public:
    BaseProcessSystem(int32 runtimeComponentType, Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;

protected:
    Vector<Component*> components;
    int32 runtimeComponentType;
};
}
