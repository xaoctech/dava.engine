#include "INetworkInputSimulationSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"

#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(INetworkInputSimulationSystem)
{
    ReflectionRegistrator<INetworkInputSimulationSystem>::Begin()
    .End();
}

uint32 GetKeyboardDeviceId()
{
    uint32 keyboardId = ~0;
    DeviceManager* deviceManager = GetEngineContext()->deviceManager;
    if (nullptr != deviceManager && nullptr != deviceManager->GetKeyboard())
    {
        keyboardId = deviceManager->GetKeyboard()->GetId();
    }
    return keyboardId;
}

uint32 GetMouseDeviceId()
{
    uint32 mouseId = ~0;
    DeviceManager* deviceManager = GetEngineContext()->deviceManager;
    if (nullptr != deviceManager && nullptr != deviceManager->GetMouse())
    {
        mouseId = deviceManager->GetMouse()->GetId();
    }
    return mouseId;
}

INetworkInputSimulationSystem::INetworkInputSimulationSystem(Scene* scene, const ComponentMask& requiredComponents)
    : BaseSimulationSystem(scene, requiredComponents)
{
}

void INetworkInputSimulationSystem::ReSimulationStart(Entity* entity, uint32 frameId)
{
    NetworkInputComponent* netInputComp = entity->GetComponent<NetworkInputComponent>();
    NetworkInputComponent::ResimulationCache& cache = netInputComp->ModifyResimulationCache();
    if (cache.startCounter == 0)
    {
        netInputComp->SetFrameFail(frameId + 1);
    }
    ++cache.startCounter;
}

void INetworkInputSimulationSystem::Simulate(Entity* entity)
{
    NetworkInputComponent* netInputComp = entity->GetComponent<NetworkInputComponent>();
    const uint32 frameFail = netInputComp->GetFrameFail();
    DVASSERT(frameFail > 0);

    const NetworkInputComponent::History& history = netInputComp->GetHistory();
    NetworkInputComponent::ResimulationCache& cache = netInputComp->ModifyResimulationCache();

    bool inputToApplyFound = false;
    for (auto it = history.Begin(); it != history.End(); ++it)
    {
        if (it.Frame() == frameFail)
        {
            if (cache.simulationCounter == 0)
            {
                const NetworkInputComponent::Data& data = it.Data();
                cache.actions.digitalActions = NetworkInputSystem::UnpackDigitalActions(data.actions, GetScene());
                cache.actions.analogActions = NetworkInputSystem::UnpackAnalogActions(data.actions, data.analogStates, GetScene());
#ifdef DISABLE_LOSSY_PACK
                cache.actions.cameraDelta = data.cameraDelta.data;
#else
                cache.actions.cameraDelta.Unpack(data.cameraDelta.GetData());
#endif
            }

            ApplyDigitalActions(entity, cache.actions.digitalActions,
                                frameFail, NetworkTimeSingleComponent::FrameDurationS);
            ApplyAnalogActions(entity, cache.actions.analogActions,
                               frameFail, NetworkTimeSingleComponent::FrameDurationS);

            inputToApplyFound = true;

            break;
        }
    }

    if (!inputToApplyFound)
    {
        Logger::Info("INetworkInputSimulationSystem: did not find input to apply for frame %u", frameFail);
    }

    ++cache.simulationCounter;
    if (cache.simulationCounter == cache.startCounter)
    {
        netInputComp->SetFrameFail(frameFail + 1);
        cache.simulationCounter = 0;
    }
}

void INetworkInputSimulationSystem::ReSimulationEnd(Entity* entity)
{
    NetworkInputComponent* netInputComp = entity->GetComponent<NetworkInputComponent>();
    NetworkInputComponent::ResimulationCache& cache = netInputComp->ModifyResimulationCache();
    DVASSERT(cache.simulationCounter == 0);
    --cache.startCounter;
    if (cache.startCounter == 0)
    {
        netInputComp->SetFrameFail(0);
    }
}
}
