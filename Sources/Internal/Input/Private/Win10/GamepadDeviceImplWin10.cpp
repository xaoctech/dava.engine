#include "Input/Private/Win10/GamepadDeviceImplWin10.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Input/GamepadDevice.h"
#include "Input/InputElements.h"
#include "Input/Private/DigitalElement.h"

namespace DAVA
{
namespace Private
{
GamepadDeviceImpl::GamepadDeviceImpl(GamepadDevice* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadDeviceImpl::Update()
{
    using ::Windows::Gaming::Input::GamepadReading;
    using ::Windows::Gaming::Input::GamepadButtons;

    static const struct
    {
        GamepadButtons button;
        eInputElements element;
    } buttonMask[] = {
        { GamepadButtons::Menu, eInputElements::GAMEPAD_MENU },
        { GamepadButtons::A, eInputElements::GAMEPAD_A },
        { GamepadButtons::B, eInputElements::GAMEPAD_B },
        { GamepadButtons::X, eInputElements::GAMEPAD_X },
        { GamepadButtons::Y, eInputElements::GAMEPAD_Y },
        { GamepadButtons::DPadLeft, eInputElements::GAMEPAD_DPAD_LEFT },
        { GamepadButtons::DPadRight, eInputElements::GAMEPAD_DPAD_RIGHT },
        { GamepadButtons::DPadUp, eInputElements::GAMEPAD_DPAD_UP },
        { GamepadButtons::DPadDown, eInputElements::GAMEPAD_DPAD_DOWN },
        { GamepadButtons::LeftThumbstick, eInputElements::GAMEPAD_LTHUMB },
        { GamepadButtons::RightThumbstick, eInputElements::GAMEPAD_RTHUMB },
        { GamepadButtons::LeftShoulder, eInputElements::GAMEPAD_LSHOUDER },
        { GamepadButtons::RightShoulder, eInputElements::GAMEPAD_RSHOUDER },
    };

    const GamepadReading reading = gamepad->GetCurrentReading();

    for (const auto& x : buttonMask)
    {
        uint32 index = x.element - eInputElements::GAMEPAD_FIRST_BUTTON;
        bool pressed = (reading.Buttons & x.button) != GamepadButtons::None;
        DigitalInputElement di(gamepadDevice->buttons[index]);
        if (pressed != di.IsPressed())
        {
            gamepadDevice->buttonChangedMask.set(index);
        }
        pressed ? di.Press() : di.Release();
    }

    auto handleAxis = [this](eInputElements element, float32 newValue) {
        uint32 index = element - eInputElements::GAMEPAD_FIRST_AXIS;
        if (newValue != gamepadDevice->axises[index].x)
        {
            gamepadDevice->axises[index].x = newValue;
            gamepadDevice->axisChangedMask.set(index);
        }
    };

    handleAxis(eInputElements::GAMEPAD_LTHUMB_X, static_cast<float32>(reading.LeftThumbstickX));
    handleAxis(eInputElements::GAMEPAD_LTHUMB_Y, static_cast<float32>(reading.LeftThumbstickY));
    handleAxis(eInputElements::GAMEPAD_RTHUMB_X, static_cast<float32>(reading.RightThumbstickX));
    handleAxis(eInputElements::GAMEPAD_RTHUMB_Y, static_cast<float32>(reading.RightThumbstickY));
    handleAxis(eInputElements::GAMEPAD_LTRIGGER, static_cast<float32>(reading.LeftTrigger));
    handleAxis(eInputElements::GAMEPAD_RTRIGGER, static_cast<float32>(reading.RightTrigger));
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    // Only one connected gamepad is supported by dava.engine for now.
    // So choose first gamepad from gamepad list.

    using ::Windows::Foundation::Collections::IVectorView;
    using ::Windows::Gaming::Input::Gamepad;

    if (gamepad == nullptr)
    {
        IVectorView<Gamepad ^> ^ gamepads = Gamepad::Gamepads;
        if (gamepads->Size != 0)
        {
            gamepad = gamepads->GetAt(0);
            gamepadDevice->profile = eGamepadProfiles::EXTENDED;
        }
    }
    return gamepad != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 /*id*/)
{
    // Only one connected gamepad is supported by dava.engine for now.
    // So check whether currently using gamepad is removed

    using ::Windows::Foundation::Collections::IVectorView;
    using ::Windows::Gaming::Input::Gamepad;

    bool removed = true;
    IVectorView<Gamepad ^> ^ gamepads = Gamepad::Gamepads;
    for (unsigned int i = 0, n = gamepads->Size; i < n; ++i)
    {
        if (gamepads->GetAt(i) == gamepad)
        {
            removed = false;
            break;
        }
    }
    if (removed)
    {
        gamepad = nullptr;
    }
    return gamepad != nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
