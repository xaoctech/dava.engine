#include "Input/Private/ActionSystemImpl.h"
#include "Input/InputSystem.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"

namespace DAVA
{
namespace Private
{
// Calculates number of states in array which are not empty
// Used for sorting
static size_t GetNonEmptyStatesCount(const Array<DigitalControlState, MAX_DIGITAL_STATES_COUNT>& array)
{
    size_t result = 0;
    for (const DigitalControlState& deviceControlState : array)
    {
        if (deviceControlState.controlId > 0)
        {
            ++result;
        }
    }

    return result;
}

bool DigitalBindingCompare::operator()(const DigitalBinding& first, const DigitalBinding& second) const
{
    return GetNonEmptyStatesCount(first.requiredStates) > GetNonEmptyStatesCount(second.requiredStates);
}

bool AnalogBindingCompare::operator()(const AnalogBinding& first, const AnalogBinding& second) const
{
    return GetNonEmptyStatesCount(first.requiredDigitalControlStates) > GetNonEmptyStatesCount(second.requiredDigitalControlStates);
}

ActionSystemImpl::ActionSystemImpl(ActionSystem* actionSystem) : actionSystem(actionSystem)
{
   GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &ActionSystemImpl::OnInputEvent));
}

ActionSystemImpl::~ActionSystemImpl()
{
    // TODO: unsubscribe
}

void ActionSystemImpl::BindSet(const ActionSet& set, Array<uint32, MAX_DEVICES_COUNT> devices)
{
    BindedActionSet bindedSet;
    bindedSet.digitalBindings.insert(set.digitalBindings.begin(), set.digitalBindings.end());
    bindedSet.analogBindings.insert(set.analogBindings.begin(), set.analogBindings.end());
    bindedSet.devices = devices;

    bindedSets.push_back(bindedSet);
}

// Helper function to check if specified states are active
bool ActionSystemImpl::CheckDigitalStates(const Array<DigitalControlState, MAX_DIGITAL_STATES_COUNT>& states, const Array<uint32, MAX_DEVICES_COUNT>& devices)
{
    for (const DigitalControlState& requiredState : states)
    {
        if (requiredState.controlId == 0)
        {
            break;
        }

        bool requiredStateMatches = false;

        for (const uint32 deviceId : devices)
        {
            if (deviceId == 0)
            {
                break;
            }

            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(deviceId);
            if (device->HasControlWithId(requiredState.controlId))
            {
                const eDigitalControlState state = device->GetDigitalControlState(requiredState.controlId);

                if ((state & requiredState.stateMask) == requiredState.stateMask)
                {
                    requiredStateMatches = true;
                    break;
                }
            }
        }

        if (!requiredStateMatches)
        {
            // At least one state is not in the state which is required, stop
            return false;
        }
    }

    return true;
}

bool ActionSystemImpl::OnInputEvent(const InputEvent& event)
{
    for (const BindedActionSet& setBinding : bindedSets)
    {
        // Check if any digital action has triggered
        for (auto it = setBinding.digitalBindings.begin(); it != setBinding.digitalBindings.end(); ++it)
        {
            DigitalBinding const& binding = *it;

            const bool triggered = CheckDigitalStates(binding.requiredStates, setBinding.devices);

            if (triggered)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogControlState = binding.outputAnalogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return true;
            }
        }
    }

    for (const BindedActionSet& setBinding : bindedSets)
    {
        // Check if any analog action has triggered
        for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
        {
            AnalogBinding const& binding = *it;

            if (event.controlId != binding.analogControlId)
            {
                continue;
            }

            const bool triggered = CheckDigitalStates(binding.requiredDigitalControlStates, setBinding.devices);

            if (triggered)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogControlState = event.analogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return true;
            }
        }
    }

    return false;
}
}
}