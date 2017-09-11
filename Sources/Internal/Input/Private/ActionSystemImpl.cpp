#include "Input/Private/ActionSystemImpl.h"
#include "Input/InputSystem.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"
#include "Concurrency/Thread.h"

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
    DVASSERT(Thread::IsMainThread());

    inputHandlerToken = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &ActionSystemImpl::OnInputEvent));

    Engine::Instance()->endFrame.Connect(this, &ActionSystemImpl::OnEndFrame);

    Engine::Instance()->update.Connect(this, &ActionSystemImpl::OnUpdate);
}

ActionSystemImpl::~ActionSystemImpl()
{
    DVASSERT(Thread::IsMainThread());

    GetEngineContext()->inputSystem->RemoveHandler(inputHandlerToken);

    Engine::Instance()->endFrame.Disconnect(this);

    Engine::Instance()->update.Disconnect(this);
}

void ActionSystemImpl::BindSet(const ActionSet& set, Vector<uint32> devices)
{
    DVASSERT(Thread::IsMainThread());

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

    for (const auto& digitalBinding : set.digitalBindings)
    {
        bool noneElementProcessed = false;
        for (size_t i = 0; i < digitalBinding.digitalElements.size(); ++i)
        {
            if (digitalBinding.digitalElements[i] != eInputElements::NONE)
            {
                DVASSERT(!noneElementProcessed, "eInputElements::NONE in the middle of the digital elements array.");
                DVASSERT(digitalBinding.digitalStates[i] != DigitalElementState::Released(), "Do you really want to bind an action on key release? Your desires are ... unconventional.");
            }
            else
            {
                noneElementProcessed = true;
            }
        }

        DVASSERT(digitalBinding.digitalElements[0] != eInputElements::NONE, "Digital elements array can't be empty for digital binding.");

        ActionState digitalState;
        digitalState.active = false;
        digitalState.action.actionId = digitalBinding.actionId;
        digitalState.action.analogState = digitalBinding.outputAnalogState;

        digitalActionsStates[digitalBinding.actionId] = digitalState;
    }

    for (const auto& analogBinding : set.analogBindings)
    {
        bool noneElementProcessed = false;
        for (size_t i = 0; i < analogBinding.digitalElements.size(); ++i)
        {
            if (analogBinding.digitalElements[i] != eInputElements::NONE)
            {
                DVASSERT(!noneElementProcessed, "eInputElements::NONE in the middle of the digital elements array.");
                DVASSERT(analogBinding.digitalStates[i] != DigitalElementState::Released(), "Do you really want to bind an action on key release? Your desires are ... unconventional.");
            }
            else
            {
                noneElementProcessed = true;
            }
        }

        DVASSERT(analogBinding.analogElementId != eInputElements::NONE);

        ActionState analogState;
        // Always active if there are no digital elements for this binding
        analogState.active = analogBinding.digitalElements[0] == eInputElements::NONE;
        analogState.action.actionId = analogBinding.actionId;

        analogActionsStates[analogBinding.actionId] = analogState;
    }

    BoundActionSet boundSet;
    boundSet.name = set.name;
    boundSet.digitalBindings.insert(set.digitalBindings.begin(), set.digitalBindings.end());
    boundSet.analogBindings.insert(set.analogBindings.begin(), set.analogBindings.end());
    boundSet.devices = devices;

    boundSets.emplace_back(std::move(boundSet));
}

void ActionSystemImpl::UnbindAllSets()
{
    DVASSERT(Thread::IsMainThread());

    boundSets.clear();
    analogActionsStates.clear();
    digitalActionsStates.clear();
}

bool ActionSystemImpl::GetDigitalActionState(FastName actionId) const
{
    DVASSERT(Thread::IsMainThread());

    auto digitalActionStateIter = digitalActionsStates.find(actionId);

    DVASSERT(digitalActionStateIter != digitalActionsStates.end());

    const ActionState& digitalActionState = digitalActionStateIter->second;
    return digitalActionState.active;
}

AnalogActionState ActionSystemImpl::GetAnalogActionState(FastName actionId) const
{
    DVASSERT(Thread::IsMainThread());

    auto analogActionStateIter = analogActionsStates.find(actionId);

    DVASSERT(analogActionStateIter != analogActionsStates.end());

    const ActionState& analogActionState = analogActionStateIter->second;
    return AnalogActionState(analogActionState.active, analogActionState.action.analogState);
}

// Helper function to check if specified states are active
bool ActionSystemImpl::CheckDigitalStates(const Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT>& elements, const Array<DigitalElementState, ActionSystem::MAX_DIGITAL_STATES_COUNT>& states, const Vector<uint32>& devices)
{
    for (size_t i = 0; i < ActionSystem::MAX_DIGITAL_STATES_COUNT; ++i)
    {
        eInputElements elementId = elements[i];

        // If it's an empty state, break
        if (elementId == eInputElements::NONE)
        {
            break;
        }

        const DigitalElementState requiredState = states[i];
        bool requiredStateMatches = false;

        for (const uint32 deviceId : devices)
        {
            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(deviceId);
            if (device != nullptr)
            {
                if (device->IsElementSupported(elementId))
                {
                    const DigitalElementState state = device->GetDigitalElementState(elementId);
                    if (CompareDigitalStates(requiredState, state))
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
    DVASSERT(Thread::IsMainThread());

    if (event.deviceType == eInputDeviceTypes::KEYBOARD && event.keyboardEvent.charCode > 0)
    {
        return false;
    }

    InputElementInfo eventElementInfo = GetInputElementInfo(event.elementId);

    // Check if any analog action has triggered
    if (eventElementInfo.type == eInputElementTypes::ANALOG)
    {
        for (const BoundActionSet& setBinding : boundSets)
        {
            for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
            {
                AnalogBinding const& binding = *it;

                if (event.elementId != binding.analogElementId)
                {
                    continue;
                }

                auto analogActionStateIter = analogActionsStates.find(binding.actionId);

                DVASSERT(analogActionStateIter != analogActionsStates.end());

                ActionState& analogActionState = analogActionStateIter->second;
                analogActionState.action.analogState = event.analogState;
                analogActionState.action.triggeredDevice = event.device;

                if (analogActionState.active)
                {
                    actionSystem->ActionTriggered.Emit(analogActionState.action);
                    return false; // TODO
                }
            }
        }
    }

    // Check if any digital action is active
    if (eventElementInfo.type == eInputElementTypes::DIGITAL)
    {
        for (const BoundActionSet& setBinding : boundSets)
        {
            bool digitalBindingHandled = false;
            for (auto it = setBinding.digitalBindings.begin(); it != setBinding.digitalBindings.end(); ++it)
            {
                DigitalBinding const& digitalBinding = *it;

                if (std::find(digitalBinding.digitalElements.begin(), digitalBinding.digitalElements.end(), event.elementId) == digitalBinding.digitalElements.end())
                {
                    continue;
                }

                auto digitalActionStateIter = digitalActionsStates.find(digitalBinding.actionId);

                DVASSERT(digitalActionStateIter != digitalActionsStates.end());

                ActionState& digitalActionState = digitalActionStateIter->second;
                digitalActionState.active = false;

                if (digitalBindingHandled)
                {
                    continue;
                }

                const bool triggered = CheckDigitalStates(digitalBinding.digitalElements, digitalBinding.digitalStates, setBinding.devices);

                if (triggered)
                {
                    digitalActionState.active = true;
                    digitalActionState.action.triggeredDevice = event.device;

                    digitalBindingHandled = true;
                }
            }

            // Check 'active' flag for all affected analog bindings
            for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
            {
                AnalogBinding const& analogBinding = *it;

                if (std::find(analogBinding.digitalElements.begin(), analogBinding.digitalElements.end(), event.elementId) == analogBinding.digitalElements.end())
                {
                    continue;
                }

                auto analogActionStateIter = analogActionsStates.find(analogBinding.actionId);

                DVASSERT(analogActionStateIter != analogActionsStates.end());

                ActionState& analogActionState = analogActionStateIter->second;
                analogActionState.active = CheckDigitalStates(analogBinding.digitalElements, analogBinding.digitalStates, setBinding.devices);
            }
        }
    }

    return false;
}

void ActionSystemImpl::OnUpdate(float32 elapsedTime)
{
    DVASSERT(Thread::IsMainThread());

    for (auto& p : digitalActionsStates)
    {
        ActionState& digitalState = p.second;

        if (digitalState.active)
        {
            actionSystem->ActionTriggered.Emit(digitalState.action);
        }
    }
}

// Reset all JustPressed and JustReleased events since they wont trigger OnInputEvent and state will hang active
void ActionSystemImpl::OnEndFrame()
{
    DVASSERT(Thread::IsMainThread());

    for (const BoundActionSet& setBinding : boundSets)
    {
        for (const DigitalBinding& digitalBinding : setBinding.digitalBindings)
        {
            if (!digitalActionsStates[digitalBinding.actionId].active)
            {
                continue;
            }

            for (size_t i = 0; i < digitalBinding.digitalElements.size(); ++i)
            {
                if (digitalBinding.digitalElements[i] == eInputElements::NONE)
                {
                    break;
                }

                if (digitalBinding.digitalStates[i].IsJustPressed() || digitalBinding.digitalStates[i].IsJustReleased())
                {
                    digitalActionsStates[digitalBinding.actionId].active = false;
                    break;
                }
            }
        }

        for (const AnalogBinding& analogBinding : setBinding.analogBindings)
        {
            if (!analogActionsStates[analogBinding.actionId].active)
            {
                continue;
            }

            for (size_t i = 0; i < analogBinding.digitalElements.size(); ++i)
            {
                if (analogBinding.digitalElements[i] == eInputElements::NONE)
                {
                    break;
                }

                if (analogBinding.digitalStates[i].IsJustPressed() || analogBinding.digitalStates[i].IsJustReleased())
                {
                    analogActionsStates[analogBinding.actionId].active = false;
                    break;
                }
            }
        }
    }
}

} // namespace Private
} // namespace DAVA
