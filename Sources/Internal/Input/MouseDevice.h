#pragma once

#include "Engine/EngineTypes.h"
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
class MouseInputDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    static const InputDeviceType TYPE;

    bool SupportsElement(uint32 elementId) const override;
    eDigitalElementState GetDigitalElementState(uint32 elementId) const override;
    AnalogElementState GetAnalogElementState(uint32 elementId) const override;

private:
    MouseInputDevice(uint32 id);
    ~MouseInputDevice() override;
    MouseInputDevice(const MouseInputDevice&) = delete;

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void HandleMouseClick(const Private::MainDispatcherEvent& e);
    void HandleMouseWheel(const Private::MainDispatcherEvent& e);
    void HandleMouseMove(const Private::MainDispatcherEvent& e);

    void OnEndFrame();

private:
    InputSystem* inputSystem = nullptr;

    Private::DigitalElement buttons[static_cast<size_t>(eMouseButtons::COUNT)];
    AnalogElementState mousePosition;
    AnalogElementState mouseWheelDelta;

    size_t endFrameConnectionToken;
};

} // namespace DAVA
