#include "Scene3D/Systems/SwitchSystem.h"

#include "Debug/DVAssert.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SwitchSystem)
{
    ReflectionRegistrator<SwitchSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &SwitchSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 4.0f)]
    .End();
}

SwitchSystem::SwitchSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<SwitchComponent>())
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

void SwitchSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SWITCH_SYSTEM);

    Set<Entity*>::iterator it;
    Set<Entity*>::const_iterator itEnd = updatableEntities.end();
    for (it = updatableEntities.begin(); it != itEnd; ++it)
    {
        Entity* entity = *it;
        SwitchComponent* sw = entity->GetComponent<SwitchComponent>();

        if (sw->oldSwitchIndex != sw->newSwitchIndex)
        {
            SetSwitchHierarchy(entity, sw->newSwitchIndex);

            sw->oldSwitchIndex = sw->newSwitchIndex;

            ActionComponent* actionComponent = entity->GetComponent<ActionComponent>();
            if (NULL != actionComponent)
            {
                actionComponent->StartSwitch(sw->newSwitchIndex);
            }
        }
    }

    updatableEntities.clear();
}

void SwitchSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (EventSystem::SWITCH_CHANGED == event)
    {
        updatableEntities.insert(component->GetEntity());
    }
}

void SwitchSystem::AddEntity(Entity* entity)
{
    updatableEntities.insert(entity); //need update entity when add it into scene
}

void SwitchSystem::RemoveEntity(Entity* entity)
{
    updatableEntities.erase(entity);
}

void SwitchSystem::PrepareForRemove()
{
    updatableEntities.clear();
}

void SwitchSystem::SetSwitchHierarchy(Entity* entity, int32 switchIndex)
{
    RenderObject* ro = GetRenderObject(entity);
    if (ro)
    {
        ro->SetSwitchIndex(switchIndex);
    }

    uint32 size = entity->GetChildrenCount();
    for (uint32 i = 0; i < size; ++i)
    {
        SetSwitchHierarchy(entity->GetChild(i), switchIndex);
    }
}
}
