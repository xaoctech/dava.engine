#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/KeyboardKeys.h"
#include "Input/Private/DigitalElement.h"

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

    bool SupportsElement(uint32 elementId) const override;
    eDigitalElementState GetDigitalElementState(uint32 elementId) const override;
    AnalogElementState GetAnalogElementState(uint32 elementId) const override;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    void ProcessInputEvent(InputEvent& event);
    void OnEndFrame();

private:
    Array<Private::DigitalElement, static_cast<uint32>(eInputElements::KB_COUNT)> keys;
    size_t endFrameConnectionToken;
};
}