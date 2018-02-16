#include "Entity/SceneSystem.h"
#include "Entity/ComponentUtils.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SceneSystem)
{
    ReflectionRegistrator<SceneSystem>::Begin()
    .End();
}

SceneSystem::SceneSystem(Scene* scene_, const ComponentMask& requiredComponents_)
    : scene(scene_)
    , requiredComponents(requiredComponents_)
{
}

void SceneSystem::RegisterEntity(Entity* entity)
{
    bool needAdd = (requiredComponents & entity->GetAvailableComponentMask()) == requiredComponents;

    if (needAdd)
        this->AddEntity(entity);
}

void SceneSystem::UnregisterEntity(Entity* entity)
{
    bool needRemove = (requiredComponents & entity->GetAvailableComponentMask()) == requiredComponents;

    if (needRemove)
        this->RemoveEntity(entity);
}

bool SceneSystem::IsEntityComponentFitsToSystem(Entity* entity, Component* component)
{
    const ComponentMask& entityComponentMask = entity->GetAvailableComponentMask();
    const ComponentMask& componentsRequiredBySystem = this->GetRequiredComponents();
    ComponentMask componentToCheckType = ComponentUtils::MakeMask(component->GetType());

    bool isAllRequiredComponentsAvailable = (entityComponentMask & componentsRequiredBySystem) == componentsRequiredBySystem;
    bool isComponentMarkedForCheckAvailable = (componentsRequiredBySystem & componentToCheckType) == componentToCheckType;

    return (isAllRequiredComponentsAvailable && isComponentMarkedForCheckAvailable);
}

void SceneSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            AddEntity(entity);
        }
        else
        {
            AddComponent(entity, component);
        }
    }
}

void SceneSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (IsEntityComponentFitsToSystem(entity, component))
    {
        if (entity->GetComponentCount(component->GetType()) == 1)
        {
            RemoveEntity(entity);
        }
        else
        {
            RemoveComponent(entity, component);
        }
    }
}

void SceneSystem::RegisterSingleComponent(Component* component)
{
}

void SceneSystem::UnregisterSingleComponent(Component* component)
{
}

void SceneSystem::AddEntity(Entity* entity)
{
}

void SceneSystem::RemoveEntity(Entity* entity)
{
}

void SceneSystem::AddComponent(Entity* entity, Component* component)
{
}

void SceneSystem::RemoveComponent(Entity* entity, Component* component)
{
}

void SceneSystem::SceneDidLoaded()
{
}

void SceneSystem::ImmediateEvent(Component* component, uint32 event)
{
}

void SceneSystem::Process(float32 timeElapsed)
{
}

void SceneSystem::SetLocked(bool locked_)
{
    locked = locked_;
}

bool SceneSystem::IsLocked() const
{
    return locked;
}

void SceneSystem::SetScene(Scene* scene_)
{
    scene = scene_;
}

void SceneSystem::InputCancelled(UIEvent* event)
{
}
}
