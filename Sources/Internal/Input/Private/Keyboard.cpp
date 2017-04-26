#include "Input/Keyboard.h"

#if defined(__DAVAENGINE_WIN32__)
#include "Input/Private/Win32/KeyboardImplWin32.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Input/Private/Win10/KeyboardImplWin10.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Input/Private/Mac/KeyboardImplMac.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Input/Private/Android/KeyboardImplAndroid.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Input/Private/Ios/KeyboardImplIos.h"
#else
#error "KeyboardDevice: unknown platform"
#endif

#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputSystem.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
Keyboard::Keyboard(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , impl(new Private::KeyboardImpl())
    , keys{}
{
    Engine* engine = Engine::Instance();
    engine->endFrame.Connect(this, &Keyboard::OnEndFrame);
    engine->PrimaryWindow()->focusChanged.Connect(this, &Keyboard::OnWindowFocusChanged); // TODO: handle all the windows

    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &Keyboard::HandleMainDispatcherEvent));
}

Keyboard::~Keyboard()
{
    Engine* engine = Engine::Instance();
    engine->endFrame.Disconnect(this);
    engine->PrimaryWindow()->focusChanged.Disconnect(this);

    Private::EngineBackend::Instance()->UninstallEventFilter(this);
}

bool Keyboard::IsElementSupported(eInputElements elementId) const
{
    return IsKeyboardInputElement(elementId);
}

DigitalElementState Keyboard::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(IsElementSupported(elementId));
    return keys[elementId - eInputElements::KB_FIRST];
}

AnalogElementState Keyboard::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(false, "KeyboardInputDevice does not support analog elements");
    return {};
}

WideString Keyboard::TranslateElementToWideString(eInputElements elementId) const
{
    DVASSERT(IsElementSupported(elementId));
    return impl->TranslateElementToWideString(elementId);
}

uint32 Keyboard::GetElementNativeScancode(eInputElements elementId) const
{
    DVASSERT(IsElementSupported(elementId));
    return impl->ConvertDavaScancodeToNativeScancode(elementId);
}

void Keyboard::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    for (DigitalElementState& keyState : keys)
    {
        keyState.OnEndFrame();
    }
}

void Keyboard::OnWindowFocusChanged(DAVA::Window* window, bool focused)
{
    // Reset keyboard state when window is unfocused
    if (!focused)
    {
        int64 timestamp = SystemTimer::GetMs();
        for (size_t i = 0; i < INPUT_ELEMENTS_KB_COUNT; ++i)
        {
            DigitalElementState& keyState = keys[i];
            if (keyState.IsPressed())
            {
                keyState.Release();

                // Generate release event
                eInputElements elementId = static_cast<eInputElements>(eInputElements::KB_FIRST + i);
                CreateAndSendKeyInputEvent(elementId, keyState, window, timestamp);
            }
        }
    }
}

bool Keyboard::HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    if (e.type == MainDispatcherEvent::KEY_DOWN || e.type == MainDispatcherEvent::KEY_UP)
    {
        eInputElements elementId = impl->ConvertNativeScancodeToDavaScancode(e.keyEvent.key);
        if (elementId == eInputElements::NONE)
        {
            DVASSERT(false, "Couldn't map native scancode to dava scancode");
            return false;
        }

        // Update element state

        DigitalElementState& keyState = keys[elementId - eInputElements::KB_FIRST];
        if (e.type == MainDispatcherEvent::KEY_DOWN)
        {
            keyState.Press();
        }
        else
        {
            keyState.Release();
        }

        // Send event

        CreateAndSendKeyInputEvent(elementId, keyState, e.window, e.timestamp);

        return true;
    }
    else if (e.type == MainDispatcherEvent::KEY_CHAR)
    {
        CreateAndSendCharInputEvent(e.keyEvent.key, e.keyEvent.isRepeated, e.window, e.timestamp);

        return true;
    }

    return false;
}

void Keyboard::CreateAndSendKeyInputEvent(eInputElements elementId, DigitalElementState state, Window* window, int64 timestamp)
{
    InputEvent inputEvent;
    inputEvent.window = window;
    inputEvent.timestamp = static_cast<float64>(timestamp / 1000.0f);
    inputEvent.deviceType = eInputDeviceTypes::KEYBOARD;
    inputEvent.device = this;
    inputEvent.digitalState = state;
    inputEvent.elementId = elementId;
    inputEvent.keyboardEvent.charCode = 0;

    inputSystem->DispatchInputEvent(inputEvent);
}

void Keyboard::CreateAndSendCharInputEvent(char32_t charCode, bool charRepeated, Window* window, int64 timestamp)
{
    DVASSERT(charCode > 0);

    InputEvent inputEvent;
    inputEvent.window = window;
    inputEvent.timestamp = static_cast<float64>(timestamp / 1000.0f);
    inputEvent.deviceType = eInputDeviceTypes::KEYBOARD;
    inputEvent.device = this;
    inputEvent.elementId = eInputElements::NONE;
    inputEvent.keyboardEvent.charCode = charCode;
    inputEvent.keyboardEvent.charRepeated = charRepeated;

    inputSystem->DispatchInputEvent(inputEvent);
}

} // namespace DAVA
