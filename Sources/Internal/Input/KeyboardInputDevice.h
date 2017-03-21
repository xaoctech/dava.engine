#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/KeyboardKeys.h"
#include "Input/Private/DigitalControl.h"

namespace DAVA
{
/**
    \ingroup input
    Represents keyboard input device.
*/
class KeyboardInputDevice final : public InputDevice
{
    friend class Window; // For passing input events
    friend class DeviceManager; // For creation

public:
    static const InputDeviceType TYPE;

    eDigitalControlState GetDigitalControlState(uint32 controlId) const override;
    AnalogControlState GetAnalogControlState(uint32 controlId) const override;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    void ProcessInputEvent(InputEvent& event);
    void OnEndFrame();

private:
    Array<Private::DigitalControl, static_cast<uint32>(eKeyboardKey::TOTAL_KEYS_COUNT)> keys;
    size_t endFrameConnectionToken;
};
}