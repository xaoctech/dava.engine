#include "Input/KeyboardInputDevice.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"

namespace DAVA
{
const InputDeviceType KeyboardInputDevice::TYPE = 1;

KeyboardInputDevice::KeyboardInputDevice(uint32 id)
    : InputDevice(id)
{
    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &KeyboardInputDevice::OnEndFrame);
}

KeyboardInputDevice::~KeyboardInputDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
}

bool KeyboardInputDevice::SupportsElement(uint32 elementId) const
{
    return (elementId >= static_cast<uint32>(eInputElements::KB_FIRST)) && (elementId <= static_cast<uint32>(eInputElements::KB_LAST));
}

eDigitalElementState KeyboardInputDevice::GetDigitalElementState(uint32 elementId) const
{
    DVASSERT(SupportsElement(elementId));
    return keys[elementId].GetState();
}

AnalogElementState KeyboardInputDevice::GetAnalogElementState(uint32 elementId) const
{
    DVASSERT(false, "KeyboardInputDevice does not support analog element");
    return {};
}

void KeyboardInputDevice::ProcessInputEvent(InputEvent& event)
{
    if (!event.keyboardEvent.isCharEvent)
    {
        if ((event.digitalState & eDigitalElementState::PRESSED) != eDigitalElementState::NONE)
        {
            keys[event.elementId].Press();
        }
        else
        {
            keys[event.elementId].Release();
        }

        event.digitalState = keys[event.elementId].GetState();
    }

    // TODO: char event?

    GetEngineContext()->inputSystem->ProcessInputEvent(event);
}

void KeyboardInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (int i = 0; i < static_cast<uint32>(eInputElements::KB_COUNT); ++i)
    {
        keys[i].OnEndFrame();
    }
}
}