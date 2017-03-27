#include "Input/MouseInputDevice.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Input/InputElements.h"

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

bool MouseInputDevice::SupportsElement(uint32 elementId) const
{
    return (elementId >= eInputElements::MOUSE_FIRST) && (elementId <= eInputElements::MOUSE_LAST);
}

eDigitalElementState MouseInputDevice::GetDigitalElementState(uint32 elementId) const
{
    DVASSERT(SupportsElement(elementId));
    return GetDigitalElement(elementId)->GetState();
}

AnalogElementState MouseInputDevice::GetAnalogElementState(uint32 elementId) const
{
    DVASSERT(elementId == eInputElements::MOUSE_POSITION);
    return mousePosition;
}

void MouseInputDevice::ProcessInputEvent(InputEvent& event)
{
    if (event.elementId == eInputElements::MOUSE_POSITION)
    {
        mousePosition = event.analogState;
    }
    else
    {
        Private::DigitalElement* element = GetDigitalElement(event.elementId);
        if ((event.digitalState & eDigitalElementState::PRESSED) != eDigitalElementState::NONE)
        {
            element->Press();
        }
        else
        {
            element->Release();
        }

        event.digitalState = element->GetState();
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

Private::DigitalElement* MouseInputDevice::GetDigitalElement(DAVA::uint32 element)
{
    return const_cast<Private::DigitalElement*>(static_cast<const MouseInputDevice*>(this)->GetDigitalElement(element));
}

const Private::DigitalElement* MouseInputDevice::GetDigitalElement(DAVA::uint32 element) const
{
    switch (element)
    {
    case eInputElements::MOUSE_LBUTTON:
        return &leftButton;

    case eInputElements::MOUSE_MBUTTON:
        return &middleButton;

    case eInputElements::MOUSE_RBUTTON:
        return &rightButton;

    default:
        DVASSERT(false);
        return nullptr;
    }
}
}