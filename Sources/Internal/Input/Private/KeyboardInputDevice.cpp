#include "Input/KeyboardInputDevice.h"

#if defined(__DAVAENGINE_WIN32__)
#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Input/Private/Win10/KeyboardDeviceImplWin10.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Input/Private/Mac/KeyboardDeviceImplMac.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Input/Private/Android/KeyboardDeviceImplAndroid.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Input/Private/Ios/KeyboardDeviceImplIos.h"
#else
#error "DeviceManager: unknown platform"
#endif

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
KeyboardInputDevice::KeyboardInputDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , impl(new Private::KeyboardDeviceImpl())
{
    Engine* engine = Engine::Instance();

    endFrameConnectionToken = engine->endFrame.Connect(this, &KeyboardInputDevice::OnEndFrame);

    // TODO: handle all the windows
    primaryWindowFocusChangedToken = engine->PrimaryWindow()->focusChanged.Connect(this, &KeyboardInputDevice::OnWindowFocusChanged);

    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &KeyboardInputDevice::HandleEvent));
}

KeyboardInputDevice::~KeyboardInputDevice()
{
    Engine* engine = Engine::Instance();
    engine->endFrame.Disconnect(endFrameConnectionToken);
    engine->PrimaryWindow()->focusChanged.Disconnect(primaryWindowFocusChangedToken);

    Private::EngineBackend::Instance()->UninstallEventFilter(this);

    if (impl != nullptr)
    {
        delete impl;
        impl = nullptr;
    }
}

bool KeyboardInputDevice::SupportsElement(eInputElements elementId) const
{
    return IsKeyboardInputElement(elementId);
}

eDigitalElementStates KeyboardInputDevice::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(SupportsElement(elementId));

    // Since state corresponds to scancode keys, map virtual to scancode if necessary
    if (IsKeyboardVirtualInputElement(elementId))
    {
        elementId = ConvertVirtualToScancode(elementId);
    }

    return keys[elementId - eInputElements::KB_FIRST_SCANCODE].GetState();
}

AnalogElementState KeyboardInputDevice::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(false, "KeyboardInputDevice does not support analog elements");
    return {};
}

eInputElements KeyboardInputDevice::ConvertScancodeToVirtual(eInputElements scancodeElement) const
{
    return impl->ConvertDavaScancodeToDavaVirtual(scancodeElement);
}

eInputElements KeyboardInputDevice::ConvertVirtualToScancode(eInputElements virtualElement) const
{
    return impl->ConvertDavaVirtualToDavaScancode(virtualElement);
}

void KeyboardInputDevice::CreateAndSendInputEvent(eInputElements elementId, const Private::DigitalElement& element, Window* window, int64 timestamp) const
{
    InputEvent inputEvent;
    inputEvent.window = window;
    inputEvent.timestamp = static_cast<float64>(timestamp / 1000.0f);
    inputEvent.deviceType = eInputDeviceTypes::KEYBOARD;
    inputEvent.deviceId = GetId();
    inputEvent.digitalState = element.GetState();
    inputEvent.elementId = elementId;

    inputSystem->DispatchInputEvent(inputEvent);
}

bool KeyboardInputDevice::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    if (e.type == MainDispatcherEvent::KEY_DOWN || e.type == MainDispatcherEvent::KEY_UP)
    {
        // Save state

        eInputElements scancodeElementId = impl->ConvertNativeScancodeToDavaScancode(e.keyEvent.key);
        if (scancodeElementId != eInputElements::NONE)
        {
            Private::DigitalElement& element = keys[scancodeElementId - eInputElements::KB_FIRST_SCANCODE];

            if (e.type == MainDispatcherEvent::KEY_DOWN)
            {
                element.Press();
            }
            else
            {
                element.Release();
            }

            // Send event

            CreateAndSendInputEvent(scancodeElementId, element, e.window, e.timestamp);

            return true;
        }
    }

    return false;
}

void KeyboardInputDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    // TODO: optimize?

    for (size_t i = 0; i < INPUT_ELEMENTS_KB_COUNT_SCANCODE; ++i)
    {
        keys[i].OnEndFrame();
    }
}

void KeyboardInputDevice::OnWindowFocusChanged(DAVA::Window* window, bool focused)
{
    if (!focused)
    {
        for (size_t i = 0; i < INPUT_ELEMENTS_KB_COUNT_SCANCODE; ++i)
        {
            if (keys[i].IsPressed())
            {
                keys[i].Release();

                // Generate release event
                eInputElements scancodeElementId = static_cast<eInputElements>(eInputElements::KB_FIRST_SCANCODE + i);
                CreateAndSendInputEvent(scancodeElementId, keys[i], window, SystemTimer::GetMs());
            }
        }
    }
}

} // namespace DAVA
