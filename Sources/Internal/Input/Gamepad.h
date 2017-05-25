#pragma once

#include "Base/BaseTypes.h"
#include "Base/Token.h"
#include "Input/InputDevice.h"

#include <bitset>

namespace DAVA
{
class InputSystem;

namespace Private
{
class GamepadImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup input
    Class for working with gamepads.
*/
class Gamepad final : public InputDevice
{
    friend class DeviceManager;
    friend class Private::GamepadImpl;

public:
    // InputDevice overrides
    bool IsElementSupported(eInputElements elementId) const override;
    DigitalElementState GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    explicit Gamepad(uint32 id);
    ~Gamepad();

    void Update();
    void OnEndFrame();

    void HandleGamepadAdded(const Private::MainDispatcherEvent& e);
    void HandleGamepadRemoved(const Private::MainDispatcherEvent& e);

    void HandleGamepadMotion(const Private::MainDispatcherEvent& e);
    void HandleGamepadButton(const Private::MainDispatcherEvent& e);

    void HandleButtonPress(eInputElements element, bool pressed);
    void HandleBackButtonPress(bool pressed);
    void HandleAxisMovement(eInputElements element, float32 newValue, bool horizontal);

    InputSystem* inputSystem = nullptr;
    std::unique_ptr<Private::GamepadImpl> impl;

    static const uint32 BUTTON_COUNT = static_cast<uint32>(eInputElements::GAMEPAD_LAST_BUTTON - eInputElements::GAMEPAD_FIRST_BUTTON + 1);
    static const uint32 AXIS_COUNT = static_cast<uint32>(eInputElements::GAMEPAD_LAST_AXIS - eInputElements::GAMEPAD_FIRST_AXIS + 1);
    Array<DigitalElementState, BUTTON_COUNT> buttons;
    Array<AnalogElementState, AXIS_COUNT> axes;
    DigitalElementState backButton;

    std::bitset<BUTTON_COUNT> buttonChangedMask;
    std::bitset<AXIS_COUNT> axisChangedMask;
    std::bitset<BUTTON_COUNT + AXIS_COUNT> supportedElements;

    Token endFrameConnectionToken;
};

} // namespace DAVA
