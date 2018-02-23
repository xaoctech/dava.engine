#include "Scene3D/Systems/ActionCollectSystem.h"

#include "DeviceManager/DeviceManager.h"
#include "Engine/Engine.h"
#include "Input/Keyboard.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Scene3D/Scene.h"
#include "Time/SystemTimer.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ActionCollectSystem)
{
    ReflectionRegistrator<ActionCollectSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ActionCollectSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 1.0f)]
    .End();
}

ActionCollectSystem::ActionCollectSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
}

void DAVA::ActionCollectSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    // TODO: this system and whole actions mechanism seems weird. Think about some simplifications.

    for (auto& digitalAction : actionsSingleComponent->collectedDigitalActions)
    {
        devicesToActionSets[digitalAction.deviceId].digitalBindings.push_back(digitalAction.digitalBinding);
        actionsSingleComponent->AddAvailableDigitalAction(digitalAction.digitalBinding.actionId);
        BindActionSet(digitalAction.deviceId);
    }

    for (auto& analogAction : actionsSingleComponent->collectedAnalogActions)
    {
        devicesToActionSets[analogAction.deviceId].analogBindings.push_back(analogAction.analogBinding);
        actionsSingleComponent->AddAvailableAnalogAction(analogAction.analogBinding.actionId, analogAction.analogPrecision);
        BindActionSet(analogAction.deviceId);
    }

    actionsSingleComponent->collectedDigitalActions.clear();
    actionsSingleComponent->collectedAnalogActions.clear();
}

void ActionCollectSystem::OnAction(Action action)
{
    int32 localPlayerId = actionsSingleComponent->GetLocalPlayerId();
    if (localPlayerId > 0)
    {
        if (actionsSingleComponent->IsAvaildableDigitalAction(action.actionId))
        {
            actionsSingleComponent->AddDigitalAction(action.actionId, localPlayerId);
        }

        if (actionsSingleComponent->IsAvaildableAnalogAction(action.actionId))
        {
            actionsSingleComponent->AddAnalogAction(action.actionId, Vector2(action.analogState.x, action.analogState.y), localPlayerId);
        }
    }
}

void ActionCollectSystem::BindActionSet(uint32 deviceId)
{
    ActionSystem* actionSystem = GetEngineContext()->actionSystem;
    if (actionSystem)
    {
        auto findIt = devicesToActionSets.find(deviceId);
        DVASSERT(findIt != devicesToActionSets.end());
        actionSystem->BindSet(findIt->second, deviceId);
        if (!connected)
        {
            actionSystem->ActionTriggered.Connect(this, &ActionCollectSystem::OnAction);
            connected = true;
        }
    }
}

void DAVA::ActionCollectSystem::PrepareForRemove()
{
    devicesToActionSets.clear();
}
