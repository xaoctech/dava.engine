#include "Input/InputListener.h"

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
void InputListener::Listen(eInputListenerModes mode, Function<void(Vector<InputEvent>)> callback)
{
    Listen(mode, callback, 0, eInputDeviceTypes::CLASS_ALL);
}

void InputListener::Listen(eInputListenerModes mode, Function<void(Vector<InputEvent>)> callback, uint32 deviceId)
{
    Listen(mode, callback, deviceId, eInputDeviceTypes::CLASS_ALL);
}

void InputListener::Listen(eInputListenerModes mode, Function<void(Vector<InputEvent>)> callback, eInputDeviceTypes deviceTypesMask)
{
    Listen(mode, callback, 0, deviceTypesMask);
}

void InputListener::Listen(eInputListenerModes mode, Function<void(Vector<InputEvent>)> callback, uint32 deviceId, eInputDeviceTypes deviceTypesMask)
{
    DVASSERT(callback != nullptr);

    StopListening();

    currentMode = mode;
    currentCallback = callback;
    currentDeviceId = deviceId;
    currentDeviceTypesMask = deviceTypesMask;

    inputHandlerToken = GetEngineContext()->inputSystem->AddHandler(deviceTypesMask, MakeFunction(this, &InputListener::OnInputEvent));
}

bool InputListener::OnInputEvent(const InputEvent& e)
{
    // If we're restricted to the specific device,
    // and this event is from different one - ignore it
    if (currentDeviceId > 0 && e.deviceId != currentDeviceId)
    {
        return false;
    }

    bool finishedListening = false;
    bool addElementToResult = false;

    const InputElementInfo elementInfo = GetInputElementInfo(e.elementId);
    if (elementInfo.type == eInputElementTypes::DIGITAL)
    {
        const bool isSystemKey = IsKeyboardSystemInputElement(e.elementId);
        const bool isModifierKey = IsKeyboardModifierInputElement(e.elementId);

        // Ignore system keys
        if (!isSystemKey)
        {
            if ((e.digitalState & eDigitalElementStates::JUST_PRESSED) == eDigitalElementStates::JUST_PRESSED)
            {
                if (currentMode == eInputListenerModes::DIGITAL_SINGLE_WITHOUT_MODIFIERS)
                {
                    // Ignore modifiers
                    if (!isModifierKey)
                    {
                        addElementToResult = true;
                        finishedListening = true;
                    }
                }
                else if (currentMode == eInputListenerModes::DIGITAL_SINGLE_WITH_MODIFIERS)
                {
                    addElementToResult = true;

                    // Stop listening when first non-modifier key is pressed
                    if (!isModifierKey)
                    {
                        finishedListening = true;
                    }
                }
                else if (currentMode == eInputListenerModes::DIGITAL_MULTIPLE_ANY)
                {
                    addElementToResult = true;
                }
            }
            else if ((e.digitalState & eDigitalElementStates::JUST_RELEASED) == eDigitalElementStates::JUST_RELEASED)
            {
                if (currentMode == eInputListenerModes::DIGITAL_MULTIPLE_ANY)
                {
                    // If a button was released, and we're listening for digital sequence, assume it's over
                    finishedListening = true;
                }
            }
        }
    }
    else
    {
        if (currentMode == eInputListenerModes::ANALOG)
        {
            addElementToResult = true;
            finishedListening = true;
        }
    }

    bool handled = false;

    // Add to result list if needed
    if (addElementToResult)
    {
        handled = true;
        result.push_back(e);
    }

    // If we're finished listening, invoke callback and reset state
    if (finishedListening)
    {
        currentCallback(result);
        StopListening();
    }

    return handled;
}

bool InputListener::IsListening() const
{
    return inputHandlerToken > 0;
}

void InputListener::StopListening()
{
    if (inputHandlerToken > 0)
    {
        GetEngineContext()->inputSystem->RemoveHandler(inputHandlerToken);
        inputHandlerToken = 0;

        result.clear();
    }
}

} // namespace DAVA
