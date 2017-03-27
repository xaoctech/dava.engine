#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/Private/DigitalControl.h"

namespace DAVA
{
/**
    \ingroup input
    Represents mouse input device.
*/
class MouseInputDevice final : public InputDevice
{
    friend class Window; // For passing input events
    friend class DeviceManager; // For creation

public:
    static const InputDeviceType TYPE;

    bool HasControlWithId(uint32 controlId) const override;
    eDigitalControlState GetDigitalControlState(uint32 controlId) const override;
    AnalogControlState GetAnalogControlState(uint32 controlId) const override;

private:
    MouseInputDevice(uint32 id);
    ~MouseInputDevice();
    MouseInputDevice(const MouseInputDevice&) = delete;
    MouseInputDevice& operator=(const MouseInputDevice&) = delete;

    void ProcessInputEvent(InputEvent& event);
    void OnEndFrame();

    Private::DigitalControl* GetDigitalControl(DAVA::uint32 controlId);
    const Private::DigitalControl* GetDigitalControl(DAVA::uint32 controlId) const;

private:
    Private::DigitalControl leftButton;
    Private::DigitalControl rightButton;
    Private::DigitalControl middleButton;
    AnalogControlState mousePosition;

    size_t endFrameConnectionToken;
};
}