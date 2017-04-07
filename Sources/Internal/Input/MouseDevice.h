#pragma once

#include "Base/Token.h"
#include "Input/InputDevice.h"

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
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

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

    eDigitalElementStates buttons[eInputElements::MOUSE_LAST_BUTTON - eInputElements::MOUSE_FIRST_BUTTON + 1];
    AnalogElementState mousePosition;
    AnalogElementState mouseWheelDelta;

    Token endFrameConnectionToken;
};

} // namespace DAVA
