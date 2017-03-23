#include "Input/MouseInputDevice.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"

namespace DAVA
{
const InputDeviceType MouseInputDevice::TYPE = 2;

MouseInputDevice::MouseInputDevice(uint32 id)
    : InputDevice(id)
{
    mousePosition.x = mousePosition.y = mousePosition.z = 0.0f;
    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &MouseInputDevice::OnEndFrame);
}

MouseInputDevice::~MouseInputDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
}

bool MouseInputDevice::HasControlWithId(uint32 controlId) const
{
    return (controlId >= eControl::LEFT_BUTTON) && (controlId <= eControl::MOUSE);
}

eDigitalControlState MouseInputDevice::GetDigitalControlState(uint32 controlId) const
{
    DVASSERT(controlId < eControl::MOUSE);
    return GetDigitalControl(controlId)->GetState();
}

AnalogControlState MouseInputDevice::GetAnalogControlState(uint32 controlId) const
{
    DVASSERT(controlId == eControl::MOUSE);
    return mousePosition;
}

void MouseInputDevice::ProcessInputEvent(InputEvent& event)
{
    if (event.controlId == MOUSE)
    {
        mousePosition = event.analogState;
    }
    else
    {
        Private::DigitalControl* control = GetDigitalControl(event.controlId);
        if ((event.digitalState & eDigitalControlState::PRESSED) != eDigitalControlState::NONE)
        {
            control->Press();
        }
        else
        {
            control->Release();
        }

        event.digitalState = control->GetState();
    }

    GetEngineContext()->inputSystem->ProcessInputEvent(event);
}

void MouseInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (int i = 0; i < static_cast<uint32>(Key::TOTAL_KEYS_COUNT); ++i)
    {
        leftButton.OnEndFrame();
        middleButton.OnEndFrame();
        rightButton.OnEndFrame();
    }
}

Private::DigitalControl* MouseInputDevice::GetDigitalControl(DAVA::uint32 controlId)
{
    return const_cast<Private::DigitalControl*>(static_cast<const MouseInputDevice*>(this)->GetDigitalControl(controlId));
}

const Private::DigitalControl* MouseInputDevice::GetDigitalControl(DAVA::uint32 controlId) const
{
    switch (controlId)
    {
    case eControl::LEFT_BUTTON:
        return &leftButton;

    case eControl::MIDDLE_BUTTON:
        return &middleButton;

    case eControl::RIGHT_BUTTON:
        return &rightButton;

    default:
        DVASSERT(false);
        return nullptr;
    }
}
}