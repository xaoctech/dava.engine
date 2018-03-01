#include "NetworkInputSimulationSystem.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"

#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkInputSimulationSystem)
{
    ReflectionRegistrator<NetworkInputSimulationSystem>::Begin()[M::Tags("network", "input", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkInputSimulationSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 14.1f)]
    .End();
}

NetworkInputSimulationSystem::NetworkInputSimulationSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkInputComponent>())
{
    asc = scene->GetSingletonComponent<ActionsSingleComponent>();
    networkTimeSingleComponent = scene->AquireSingleComponentForRead<NetworkTimeSingleComponent>();

    entityGroup = scene->AquireEntityGroup<NetworkInputComponent>();
}

void NetworkInputSimulationSystem::ReSimulationStart()
{
    // Backup.
    backUp.playerIdsToActions = std::move(asc->playerIdsToActions);
    asc->playerIdsToActions.clear();
}

void NetworkInputSimulationSystem::ProcessFixed(float32 dt)
{
    if (!IsReSimulating())
    {
        return;
    }

    uint32 frameId = networkTimeSingleComponent->GetFrameId();

    for (Entity* entity : entityGroup->GetEntities())
    {
        NetworkInputComponent* netInputComp = entity->GetComponent<NetworkInputComponent>();

        const NetworkInputComponent::History& history = netInputComp->GetHistory();

        for (auto it = history.Begin(); it != history.End(); ++it)
        {
            if (it.Frame() == frameId)
            {
                const NetworkInputComponent::Data& data = it.Data();

                ActionsSingleComponent::Actions actions;

                actions.clientFrameId = frameId;
                actions.digitalActions = NetworkInputSystem::UnpackDigitalActions(data.actions, GetScene());
                actions.analogActions = NetworkInputSystem::UnpackAnalogActions(data.actions, data.analogStates, GetScene());
#ifdef DISABLE_LOSSY_PACK
                actions.cameraDelta = data.cameraDelta.data;
#else
                actions.cameraDelta.Unpack(data.cameraDelta.GetData());
#endif
                AddActionsForClient(GetScene(), entity, std::move(actions));

                break;
            }
        }
    }
}

void NetworkInputSimulationSystem::ReSimulationEnd()
{
    // Restore.
    asc->playerIdsToActions = std::move(backUp.playerIdsToActions);
    backUp.playerIdsToActions.clear();
}
}
