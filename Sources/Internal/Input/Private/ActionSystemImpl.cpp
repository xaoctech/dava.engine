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
static size_t GetNonEmptyStatesCount(const Array<DigitalElementState, MAX_DIGITAL_STATES_COUNT>& array)
{
    size_t result = 0;
    for (const DigitalElementState& deviceControlState : array)
    {
        if (deviceControlState.elementId > 0)
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
    return GetNonEmptyStatesCount(first.requiredDigitalElementStates) > GetNonEmptyStatesCount(second.requiredDigitalElementStates);
}

ActionSystemImpl::ActionSystemImpl(ActionSystem* actionSystem)
    : actionSystem(actionSystem)
{
    inputHandlerToken = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &ActionSystemImpl::OnInputEvent));
}

ActionSystemImpl::~ActionSystemImpl()
{
    GetEngineContext()->inputSystem->RemoveHandler(inputHandlerToken);
}

void ActionSystemImpl::BindSet(const ActionSet& set, Vector<uint32> devices)
{
    // Check if there are sets which are already bound to any of these devices
    // Unbind them from these devices if there are
    if (devices.size() > 0)
    {
        auto iter = boundSets.begin();
        while (iter != boundSets.end())
        {
            BoundActionSet& boundSet = *iter;

            // If set is bound to to specific devices
            if (boundSet.devices.size() > 0)
            {
                // Unbind it
                for (const uint32 deviceId : devices)
                {
                    boundSet.devices.erase(
                    std::remove(boundSet.devices.begin(), boundSet.devices.end(), deviceId),
                    boundSet.devices.end());
                }

                // If it is not bound to any devices anymore - remove it from the list
                if (boundSet.devices.size() == 0)
                {
                    iter = boundSets.erase(iter);
                    continue;
                }
            }

            ++iter;
        }
    }

    BoundActionSet boundSet;
    boundSet.digitalBindings.insert(set.digitalBindings.begin(), set.digitalBindings.end());
    boundSet.analogBindings.insert(set.analogBindings.begin(), set.analogBindings.end());
    boundSet.devices = devices;

    boundSets.push_back(boundSet);
}

// Helper function to check if specified states are active
bool ActionSystemImpl::CheckDigitalStates(const Array<DigitalElementState, MAX_DIGITAL_STATES_COUNT>& states, const Vector<uint32>& devices)
{
    for (const DigitalElementState& requiredState : states)
    {
        if (requiredState.elementId == 0)
        {
            break;
        }

        bool requiredStateMatches = false;

        for (const uint32 deviceId : devices)
        {
            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(deviceId);
            if (device != nullptr)
            {
                if (device->SupportsElement(requiredState.elementId))
                {
                    const eDigitalElementStates state = device->GetDigitalElementState(requiredState.elementId);

                    if ((state & requiredState.stateMask) == requiredState.stateMask)
                    {
                        requiredStateMatches = true;
                        break;
                    }
                }
            }
        }

        if (!requiredStateMatches)
        {
            // At least one control is not in the state which is required, stop
            return false;
        }
    }

    return true;
}

bool ActionSystemImpl::OnInputEvent(const InputEvent& event)
{
    // Handle Bound sets

    for (const BoundActionSet& setBinding : boundSets)
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
                action.analogState = binding.outputAnalogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return false; // TODO
            }
        }
    }

    for (const BoundActionSet& setBinding : boundSets)
    {
        // Check if any analog action has triggered
        for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
        {
            AnalogBinding const& binding = *it;

            if (event.elementId != binding.analogElementId)
            {
                continue;
            }

            const bool triggered = CheckDigitalStates(binding.requiredDigitalElementStates, setBinding.devices);

            if (triggered)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogState = event.analogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return false; // TODO
            }
        }
    }

    return false;
}
}
}