#include "Input/KeyboardInputDevice.h"

#if defined(__DAVAENGINE_WIN32__)
#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"
#else
#error "DeviceManager: unknown platform"
#endif

#include "Engine/Engine.h"
#include "Input/InputSystem.h"

namespace DAVA
{
KeyboardInputDevice::KeyboardInputDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , impl(new Private::KeyboardDeviceImpl())
{
    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &KeyboardInputDevice::OnEndFrame);
    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &KeyboardInputDevice::HandleEvent));
}

KeyboardInputDevice::~KeyboardInputDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
    Private::EngineBackend::Instance()->UninstallEventFilter(this);
}

bool KeyboardInputDevice::SupportsElement(eInputElements elementId) const
{
    return (elementId >= static_cast<uint32>(eInputElements::KB_FIRST)) && (elementId <= static_cast<uint32>(eInputElements::KB_LAST));
}

eDigitalElementStates KeyboardInputDevice::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(SupportsElement(elementId));

    if (elementId >= eInputElements::KB_FIRST_VIRTUAL && elementId <= eInputElements::KB_LAST_VIRTUAL)
    {
        elementId = ConvertVirtualToScancode(elementId);
    }

    return keys[elementId - eInputElements::KB_FIRST_SCANCODE].GetState();
}

AnalogElementState KeyboardInputDevice::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(false, "KeyboardInputDevice does not support analog element");
    return {};
}

bool KeyboardInputDevice::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    if (e.type == MainDispatcherEvent::KEY_DOWN || e.type == MainDispatcherEvent::KEY_UP)
    {
        // Save state

        eInputElements scancodeElement = impl->ConvertNativeScancodeToDavaScancode(e.keyEvent.key);
        Private::DigitalElement& element = keys[scancodeElement - eInputElements::KB_FIRST_SCANCODE];

        if (e.type == MainDispatcherEvent::KEY_DOWN)
        {
            element.Press();
        }
        else
        {
            element.Release();
        }

        // Send input event

        InputEvent inputEvent;
        inputEvent.window = e.window;
        inputEvent.timestamp = e.timestamp / 1000.0f;
        inputEvent.deviceType = eInputDeviceTypes::KEYBOARD;
        inputEvent.deviceId = GetId();
        inputEvent.digitalState = element.GetState();
        inputEvent.elementId = scancodeElement;

        inputSystem->DispatchInputEvent(inputEvent);

        return true;
    }
    else
    {
        return false;
    }
}

void KeyboardInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (int i = 0; i < static_cast<uint32>(eInputElements::KB_COUNT_SCANCODE); ++i)
    {
        keys[i].OnEndFrame();
    }
}

eInputElements KeyboardInputDevice::ConvertScancodeToVirtual(eInputElements scancodeElement) const
{
    return impl->ConvertDavaScancodeToDavaVirtual(scancodeElement);
}

eInputElements KeyboardInputDevice::ConvertVirtualToScancode(eInputElements virtualElement) const
{
    return impl->ConvertDavaVirtualToDavaScancode(virtualElement);
}
}