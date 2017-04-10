#include "Input/GamepadDevice.h"

#include <algorithm>

#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputEvent.h"
#include "Input/InputSystem.h"
#include "Input/Private/DIElementWrapper.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Input/Private/Android/GamepadDeviceImplAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Input/Private/Win10/GamepadDeviceImplWin10.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Input/Private/Win32/GamepadDeviceImplWin32.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Input/Private/Mac/GamepadDeviceImplMac.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Input/Private/Ios/GamepadDeviceImplIos.h"
#else
#error "GamepadDevice: unknown platform"
#endif

namespace DAVA
{
GamepadDevice::GamepadDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , impl(new Private::GamepadDeviceImpl(this))
{
    std::fill(std::begin(buttons), std::end(buttons), eDigitalElementStates::NONE);
    std::fill(std::begin(axes), std::end(axes), AnalogElementState());

    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &GamepadDevice::OnEndFrame);
}

GamepadDevice::~GamepadDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
}

bool GamepadDevice::SupportsElement(eInputElements elementId) const
{
    DVASSERT(IsGamepadAxisInputElement(elementId) || IsGamepadButtonInputElement(elementId));
    return supportedElements[elementId - eInputElements::GAMEPAD_FIRST];
}

eDigitalElementStates GamepadDevice::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(IsGamepadButtonInputElement(elementId));
    return buttons[elementId - eInputElements::GAMEPAD_FIRST_BUTTON];
}

AnalogElementState GamepadDevice::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(IsGamepadAxisInputElement(elementId));
    return axes[elementId - eInputElements::GAMEPAD_FIRST_AXIS];
}

void GamepadDevice::Update()
{
    impl->Update();

    Window* window = GetPrimaryWindow();
    for (uint32 i = eInputElements::GAMEPAD_FIRST_BUTTON; i <= eInputElements::GAMEPAD_LAST_BUTTON; ++i)
    {
        uint32 index = i - eInputElements::GAMEPAD_FIRST_BUTTON;
        if (buttonChangedMask[index])
        {
            InputEvent inputEvent;
            inputEvent.window = window;
            inputEvent.deviceType = eInputDeviceTypes::CLASS_GAMEPAD;
            inputEvent.deviceId = GetId();
            inputEvent.elementId = static_cast<eInputElements>(i);
            inputEvent.digitalState = buttons[index];
            inputSystem->DispatchInputEvent(inputEvent);
        }
    }
    buttonChangedMask.reset();

    for (uint32 i = eInputElements::GAMEPAD_FIRST_AXIS; i <= eInputElements::GAMEPAD_LAST_AXIS; ++i)
    {
        uint32 index = i - eInputElements::GAMEPAD_FIRST_AXIS;
        if (axisChangedMask[index])
        {
            InputEvent inputEvent;
            inputEvent.window = window;
            inputEvent.deviceType = eInputDeviceTypes::CLASS_GAMEPAD;
            inputEvent.deviceId = GetId();
            inputEvent.elementId = static_cast<eInputElements>(i);
            inputEvent.analogState = axes[index];
            inputSystem->DispatchInputEvent(inputEvent);
        }
    }
    axisChangedMask.reset();
}

void GamepadDevice::OnEndFrame()
{
    for (DIElementWrapper di : buttons)
    {
        di.OnEndFrame();
    }
}

void GamepadDevice::HandleGamepadAdded(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    impl->HandleGamepadAdded(deviceId);
}

void GamepadDevice::HandleGamepadRemoved(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    impl->HandleGamepadRemoved(deviceId);
}

void GamepadDevice::HandleGamepadMotion(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadMotion(e);
}

void GamepadDevice::HandleGamepadButton(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadButton(e);
}

void GamepadDevice::HandleButtonPress(eInputElements element, bool pressed)
{
    uint32 index = element - eInputElements::GAMEPAD_FIRST_BUTTON;
    DIElementWrapper di(buttons[index]);
    if (di.IsPressed() != pressed)
    {
        pressed ? di.Press() : di.Release();
        buttonChangedMask.set(index);
    }
}

void GamepadDevice::HandleAxisMovement(eInputElements element, float32 newValue, bool horizontal)
{
    uint32 index = element - eInputElements::GAMEPAD_FIRST_AXIS;
    if (horizontal)
    {
        if (newValue != axes[index].x)
        {
            axes[index].x = newValue;
            axisChangedMask.set(index);
        }
    }
    else
    {
        if (newValue != axes[index].y)
        {
            axes[index].y = newValue;
            axisChangedMask.set(index);
        }
    }
}

} // namespace DAVA
