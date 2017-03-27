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

bool KeyboardInputDevice::HasControlWithId(uint32 controlId) const
{
    return (controlId >= static_cast<uint32>(eInputControl::KB_FIRST)) && (controlId <= static_cast<uint32>(eInputControl::KB_LAST));
}

eDigitalControlState KeyboardInputDevice::GetDigitalControlState(uint32 controlId) const
{
    DVASSERT(HasControlWithId(controlId));
    return keys[controlId].GetState();
}

AnalogControlState KeyboardInputDevice::GetAnalogControlState(uint32 controlId) const
{
    DVASSERT(false, "KeyboardInputDevice does not support analog controls");
    return {};
}

void KeyboardInputDevice::ProcessInputEvent(InputEvent& event)
{
    if (!event.keyboardEvent.isCharEvent)
    {
        if ((event.digitalState & eDigitalControlState::PRESSED) != eDigitalControlState::NONE)
        {
            keys[event.controlId].Press();
        }
        else
        {
            keys[event.controlId].Release();
        }

        event.digitalState = keys[event.controlId].GetState();
    }

    // TODO: char event?

    GetEngineContext()->inputSystem->ProcessInputEvent(event);
}

void KeyboardInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (int i = 0; i < static_cast<uint32>(eInputControl::KB_COUNT); ++i)
    {
        keys[i].OnEndFrame();
    }
}
}