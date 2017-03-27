#include "Input/MouseInputDevice.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Input/InputControls.h"

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
    return (controlId >= eInputControl::MOUSE_FIRST) && (controlId <= eInputControl::MOUSE_LAST);
}

eDigitalControlState MouseInputDevice::GetDigitalControlState(uint32 controlId) const
{
    DVASSERT(HasControlWithId(controlId));
    return GetDigitalControl(controlId)->GetState();
}

AnalogControlState MouseInputDevice::GetAnalogControlState(uint32 controlId) const
{
    DVASSERT(controlId == eInputControl::MOUSE_POSITION);
    return mousePosition;
}

void MouseInputDevice::ProcessInputEvent(InputEvent& event)
{
    if (event.controlId == eInputControl::MOUSE_POSITION)
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
    case eInputControl::MOUSE_LBUTTON:
        return &leftButton;

    case eInputControl::MOUSE_MBUTTON:
        return &middleButton;

    case eInputControl::MOUSE_RBUTTON:
        return &rightButton;

    default:
        DVASSERT(false);
        return nullptr;
    }
}
}