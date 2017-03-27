#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/Private/DigitalElement.h"

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

    bool SupportsElement(uint32 elementId) const override;
    eDigitalElementState GetDigitalElementState(uint32 elementId) const override;
    AnalogElementState GetAnalogElementState(uint32 elementId) const override;

private:
    MouseInputDevice(uint32 id);
    ~MouseInputDevice();
    MouseInputDevice(const MouseInputDevice&) = delete;
    MouseInputDevice& operator=(const MouseInputDevice&) = delete;

    void ProcessInputEvent(InputEvent& event);
    void OnEndFrame();

    Private::DigitalElement* GetDigitalElement(DAVA::uint32 elementId);
    const Private::DigitalElement* GetDigitalElement(DAVA::uint32 elementId) const;

private:
    Private::DigitalElement leftButton;
    Private::DigitalElement rightButton;
    Private::DigitalElement middleButton;
    AnalogElementState mousePosition;

    size_t endFrameConnectionToken;
};
}