#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/Private/DigitalElement.h"

namespace DAVA
{
class InputSystem;
namespace Private
{
struct MainDispatcherEvent;
}

/**
    \ingroup input
    Represents mouse input device.
*/
class MouseDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    bool SupportsElement(uint32 elementId) const override;
    eDigitalElementState GetDigitalElementState(uint32 elementId) const override;
    AnalogElementState GetAnalogElementState(uint32 elementId) const override;

    eInputElements GetFirstPressedButton() const;

private:
    MouseDevice(uint32 id);
    ~MouseDevice() override;
    MouseDevice(const MouseDevice&) = delete;

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void HandleMouseClick(const Private::MainDispatcherEvent& e);
    void HandleMouseWheel(const Private::MainDispatcherEvent& e);
    void HandleMouseMove(const Private::MainDispatcherEvent& e);

    void OnEndFrame();

private:
    InputSystem* inputSystem = nullptr;

    Private::DigitalElement buttons[eInputElements::MOUSE_LAST_BUTTON - eInputElements::MOUSE_FIRST_BUTTON + 1];
    AnalogElementState mousePosition;
    AnalogElementState mouseWheelDelta;

    size_t endFrameConnectionToken;
};

} // namespace DAVA
