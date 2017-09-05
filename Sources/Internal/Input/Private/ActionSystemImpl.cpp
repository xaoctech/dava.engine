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
static size_t GetNonEmptyStatesCount(const Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT>& array)
{
    size_t result = 0;
    for (const eInputElements elementId : array)
    {
        if (elementId != eInputElements::NONE)
        {
            ++result;
        }
    }

    return result;
}

bool DigitalBindingCompare::operator()(const DigitalBinding& first, const DigitalBinding& second) const
{
    return GetNonEmptyStatesCount(first.digitalElements) > GetNonEmptyStatesCount(second.digitalElements);
}

bool AnalogBindingCompare::operator()(const AnalogBinding& first, const AnalogBinding& second) const
{
    return GetNonEmptyStatesCount(first.digitalElements) > GetNonEmptyStatesCount(second.digitalElements);
}

ActionSystemImpl::ActionSystemImpl(ActionSystem* actionSystem)
    : actionSystem(actionSystem)
{
    inputHandlerToken = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &ActionSystemImpl::OnInputEvent));

    Engine::Instance()->update.Connect(this, &ActionSystemImpl::OnUpdate);
}

ActionSystemImpl::~ActionSystemImpl()
{
    GetEngineContext()->inputSystem->RemoveHandler(inputHandlerToken);

    Engine::Instance()->update.Disconnect(this);
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

            // If set is bound to specific devices
            if (boundSet.devices.size() > 0)
            {
                // Unbind it
                for (const uint32 deviceId : devices)
                {
                    boundSet.devices.erase(std::remove(boundSet.devices.begin(), boundSet.devices.end(), deviceId), boundSet.devices.end());
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
    boundSet.name = set.name;
    boundSet.digitalBindings.insert(set.digitalBindings.begin(), set.digitalBindings.end());
    boundSet.analogBindings.insert(set.analogBindings.begin(), set.analogBindings.end());
    boundSet.devices = devices;

    for (const auto& digitalBinding : set.digitalBindings)
    {
        digitalActionsStates[digitalBinding.actionId] = {};
    }

    for (const auto& analogBinding : set.analogBindings)
    {
        analogActionsStates[analogBinding.actionId] = {};
    }

    boundSets.emplace_back(std::move(boundSet));
}

void ActionSystemImpl::UnbindAllSets()
{
    boundSets.clear();
    analogActionsStates.clear();
    digitalActionsStates.clear();
}

bool ActionSystemImpl::GetDigitalActionState(const FastName& actionId) const
{
    auto it = digitalActionsStates.find(actionId);

    DVASSERT(it != digitalActionsStates.end());

    return it->second;
}

AnalogActionState ActionSystemImpl::GetAnalogActionState(const FastName actionId) const
{
    auto it = analogActionsStates.find(actionId);

    DVASSERT(it != analogActionsStates.end());

    return it->second;
}

// Helper function to check if specified states are active
// Return 'nullptr' if at least one control is not in the state which is required and random InputDevice pointer otherwise
// This is kinda dirty, but that saves us an extra cycle in 'OnUpdate' method
InputDevice* ActionSystemImpl::CheckDigitalStates(const Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT>& elements, const Array<DigitalElementState, ActionSystem::MAX_DIGITAL_STATES_COUNT>& states, const Vector<uint32>& devices)
{
    InputDevice* ret = nullptr;

    for (size_t i = 0; i < ActionSystem::MAX_DIGITAL_STATES_COUNT; ++i)
    {
        eInputElements elementId = elements[i];

        // If it's an empty state, break
        if (elementId == eInputElements::NONE)
        {
            break;
        }

        const DigitalElementState requiredState = states[i];
        ret = nullptr;

        for (const uint32 deviceId : devices)
        {
            InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(deviceId);
            if (device != nullptr)
            {
                if (device->IsElementSupported(elementId))
                {
                    const DigitalElementState state = device->GetDigitalElementState(elementId);
                    if (CompareDigitalStates(requiredState, state))
                    {
                        ret = device;
                        break;
                    }
                }
            }
        }

        if (ret == nullptr)
        {
            // At least one control is not in the state which is required, stop
            return nullptr;
        }
    }

    return ret;
}

bool ActionSystemImpl::CompareDigitalStates(const DigitalElementState& requiredState, const DigitalElementState& state)
{
    // If an action is bound to JustPressed or JustReleased - they should match exactly for an action to be triggered only once
    // Otherwise just check for 'pressed' flag

    if (requiredState.IsJustPressed())
    {
        return state.IsJustPressed();
    }
    else if (requiredState.IsJustReleased())
    {
        return state.IsJustReleased();
    }
    else
    {
        return requiredState.IsPressed() == state.IsPressed();
    }
}

bool ActionSystemImpl::OnInputEvent(const InputEvent& event)
{
    if (event.deviceType == eInputDeviceTypes::KEYBOARD && event.keyboardEvent.charCode > 0)
    {
        return false;
    }

    InputElementInfo eventElementInfo = GetInputElementInfo(event.elementId);

    if (eventElementInfo.type == eInputElementTypes::ANALOG)
    {
        // Check if any analog action has triggered
        for (const BoundActionSet& setBinding : boundSets)
        {
            for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
            {
                AnalogBinding const& binding = *it;

                if (event.elementId != binding.analogElementId)
                {
                    continue;
                }

                InputDevice* triggered = CheckDigitalStates(binding.digitalElements, binding.digitalStates, setBinding.devices);

                analogActionsStates[binding.actionId] = { triggered != nullptr, event.analogState };

                if (triggered != nullptr)
                {
                    Action action;
                    action.actionId = binding.actionId;
                    action.analogState = event.analogState;
                    action.triggeredDevice = event.device;

                    actionSystem->ActionTriggered.Emit(action);
                }
            }
        }
    }

    return false;
}

void ActionSystemImpl::OnUpdate(float32 elapsedTime)
{
    // Set of elements that triggered an action during this update
    // Required to skip bindings with same elements
    // For example, we have two bindings:
    // - Shift + space (binding1)
    // - Space (binding2)
    // Binding1 will be processed first and if triggered, binding2 should be skipped
    static Set<eInputElements> elementsTriggeredActions;

    // Check if any digital action has triggered
    for (const BoundActionSet& setBinding : boundSets)
    {
        for (auto it = setBinding.digitalBindings.begin(); it != setBinding.digitalBindings.end(); ++it)
        {
            DigitalBinding const& binding = *it;
            digitalActionsStates[binding.actionId] = false;

            // Check if we already triggered a binding that used some elements from current binding
            // If we did, skip it

            bool skipBinding = false;
            for (const eInputElements usedElement : elementsTriggeredActions)
            {
                if (std::find(binding.digitalElements.begin(), binding.digitalElements.end(), usedElement) != binding.digitalElements.end())
                {
                    skipBinding = true;
                    break;
                }
            }

            if (skipBinding)
            {
                continue;
            }

            // Trigger if all elements are in required state

            InputDevice* triggered = CheckDigitalStates(binding.digitalElements, binding.digitalStates, setBinding.devices);

            if (triggered != nullptr)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogState = binding.outputAnalogState;
                action.triggeredDevice = triggered;

                // Remember elements that triggered that
                for (eInputElements digitalElement : binding.digitalElements)
                {
                    if (digitalElement == eInputElements::NONE)
                    {
                        break;
                    }

                    elementsTriggeredActions.insert(digitalElement);
                }

                digitalActionsStates[binding.actionId] = true;
                actionSystem->ActionTriggered.Emit(action);
            }
        }
    }

    elementsTriggeredActions.clear();
}
}
}
