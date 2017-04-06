#include "Input/Private/Win10/GamepadDeviceImplWin10.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Input/GamepadDevice.h"

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

    GamepadReading reading = gamepad->GetCurrentReading();

    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_START, (reading.Buttons & GamepadButtons::Menu) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_BACK, (reading.Buttons & GamepadButtons::View) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_A, (reading.Buttons & GamepadButtons::A) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_B, (reading.Buttons & GamepadButtons::B) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_X, (reading.Buttons & GamepadButtons::X) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_Y, (reading.Buttons & GamepadButtons::Y) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_LEFT, (reading.Buttons & GamepadButtons::DPadLeft) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_RIGHT, (reading.Buttons & GamepadButtons::DPadRight) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_UP, (reading.Buttons & GamepadButtons::DPadUp) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_DOWN, (reading.Buttons & GamepadButtons::DPadDown) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LTHUMB, (reading.Buttons & GamepadButtons::LeftThumbstick) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RTHUMB, (reading.Buttons & GamepadButtons::RightThumbstick) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LSHOUDER, (reading.Buttons & GamepadButtons::LeftShoulder) != GamepadButtons::None);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RSHOUDER, (reading.Buttons & GamepadButtons::RightShoulder) != GamepadButtons::None);

    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTHUMB_X, static_cast<float32>(reading.LeftThumbstickX));
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTHUMB_Y, static_cast<float32>(reading.LeftThumbstickY));
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTHUMB_X, static_cast<float32>(reading.RightThumbstickX));
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTHUMB_Y, static_cast<float32>(reading.RightThumbstickY));
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTRIGGER, static_cast<float32>(reading.LeftTrigger));
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTRIGGER, static_cast<float32>(reading.RightTrigger));
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
