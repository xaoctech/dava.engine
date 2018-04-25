#include "BotTaskSystem.h"

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(BotTaskSystem)
{
    ReflectionRegistrator<BotTaskSystem>::Begin()[M::SystemTags("bot", "taskbot")]
    .ConstructorByPointer<Scene*>() // TODO: system out of place. Should be in gameplay group, or be a part of fw.
    .Method("ProcessFixed", &BotTaskSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineBegin, SPI::Type::Fixed, 16.0f)]
    .End();
}

BotTaskSystem::BotTaskSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
    , taskStorage(this)
{
}

void BotTaskSystem::ProcessFixed(float32 timeElapsed)
{
    const NetworkTimeSingleComponent* networkTimeComponent = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    if (!networkTimeComponent->IsInitialized())
    {
        return;
    }

    currentDigitalActions.clear();
    currentAnalogActions.clear();

    taskStorage.DispatchTasks(timeElapsed);

    Scene* scene = GetScene();
    ActionsSingleComponent* actions = scene->GetSingleComponentForWrite<ActionsSingleComponent>(this);
    int32 localPlayerId = actions->GetLocalPlayerId();
    if (localPlayerId)
    {
        for (const FastName& action : currentDigitalActions)
        {
            actions->AddDigitalAction(action, localPlayerId);
        }

        for (auto& pair : currentAnalogActions)
        {
            actions->AddAnalogAction(pair.first, pair.second, localPlayerId);
        }
    }
}

void BotTaskSystem::RegisterComponent(Entity* entity, Component* component)
{
    taskStorage.Add(component);
}

void BotTaskSystem::UnregisterComponent(Entity* entity, Component* component)
{
    taskStorage.Remove(component);
}
