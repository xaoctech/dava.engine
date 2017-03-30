#include "Input/MouseInputDevice.h"

#include "Engine/Engine.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputElements.h"
#include "Input/InputSystem.h"

namespace DAVA
{
const InputDeviceType MouseInputDevice::TYPE = 2;

MouseInputDevice::MouseInputDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
{
    mousePosition.x = mousePosition.y = mousePosition.z = 0.0f;
    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &MouseInputDevice::OnEndFrame);
    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &MouseInputDevice::HandleEvent));
}

MouseInputDevice::~MouseInputDevice()
{
    Private::EngineBackend::Instance()->UninstallEventFilter(this);
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
}

bool MouseInputDevice::SupportsElement(uint32 elementId) const
{
    return eInputElements::MOUSE_FIRST <= elementId && elementId <= eInputElements::MOUSE_LAST;
}

eDigitalElementState MouseInputDevice::GetDigitalElementState(uint32 elementId) const
{
    DVASSERT(eInputElements::MOUSE_LBUTTON <= elementId && elementId <= eInputElements::MOUSE_EXT2BUTTON);
    return buttons[elementId - eInputElements::MOUSE_LBUTTON].GetState();
}

AnalogElementState MouseInputDevice::GetAnalogElementState(uint32 elementId) const
{
    switch (elementId)
    {
    case eInputElements::MOUSE_WHEEL:
        return mouseWheelDelta;
    case eInputElements::MOUSE_POSITION:
        return mousePosition;
    default:
        DVASSERT(0, "Invalid elementId");
        return AnalogElementState();
    }
}

bool MouseInputDevice::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    bool isHandled = true;
    switch (e.type)
    {
    case MainDispatcherEvent::MOUSE_MOVE:
        HandleMouseMove(e);
        break;
    case MainDispatcherEvent::MOUSE_BUTTON_DOWN:
    case MainDispatcherEvent::MOUSE_BUTTON_UP:
        HandleMouseClick(e);
        break;
    case MainDispatcherEvent::MOUSE_WHEEL:
        HandleMouseWheel(e);
        break;
    default:
        isHandled = false;
        break;
    }
    return isHandled;
}

void MouseInputDevice::HandleMouseClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::MOUSE_BUTTON_DOWN;
    eMouseButtons button = e.mouseEvent.button;

    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = e.timestamp / 1000.0f;
    inputEvent.deviceType = TYPE;
    inputEvent.deviceId = GetId();
    inputEvent.mouseEvent.isRelative = e.mouseEvent.isRelative;

    size_t index = static_cast<size_t>(button) - 1;
    pressed ? buttons[index].Press() : buttons[index].Release();
    inputEvent.digitalState = buttons[index].GetState();
    inputEvent.elementId = static_cast<eInputElements>(static_cast<size_t>(eInputElements::MOUSE_LBUTTON) + index);

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    inputSystem->DispatchInputEvent(inputEvent);
}

void MouseInputDevice::HandleMouseWheel(const Private::MainDispatcherEvent& e)
{
    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = e.timestamp / 1000.0f;
    inputEvent.deviceType = TYPE;
    inputEvent.deviceId = GetId();
    inputEvent.elementId = eInputElements::MOUSE_WHEEL;
    inputEvent.analogState.x = e.mouseEvent.scrollDeltaX;
    inputEvent.analogState.y = e.mouseEvent.scrollDeltaY;
    inputEvent.analogState.z = 0.f;
    inputEvent.mouseEvent.isRelative = e.mouseEvent.isRelative;

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    mouseWheelDelta.x = e.mouseEvent.scrollDeltaX;
    mouseWheelDelta.y = e.mouseEvent.scrollDeltaY;

    inputSystem->DispatchInputEvent(inputEvent);
}

void MouseInputDevice::HandleMouseMove(const Private::MainDispatcherEvent& e)
{
    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = e.timestamp / 1000.0f;
    inputEvent.deviceType = TYPE;
    inputEvent.deviceId = GetId();
    inputEvent.elementId = eInputElements::MOUSE_POSITION;
    inputEvent.analogState.x = e.mouseEvent.x;
    inputEvent.analogState.y = e.mouseEvent.y;
    inputEvent.analogState.z = 0.f;
    inputEvent.mouseEvent.isRelative = e.mouseEvent.isRelative;

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    inputSystem->DispatchInputEvent(inputEvent);
}

void MouseInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (Private::DigitalElement& b : buttons)
    {
        b.OnEndFrame();
    }
    mouseWheelDelta.x = 0.f;
    mouseWheelDelta.y = 0.f;
    mouseWheelDelta.z = 0.f;
}

} // namespace DAVA
