#pragma once

#include "Input/Private/DigitalElement.h"

namespace DAVA
{
class Window;
class InputSystem;

namespace Private
{
class KeyboardDeviceImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup input
    Represents keyboard input device.
*/
class KeyboardInputDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    // InputDevice overrides
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    void CreateAndSendInputEvent(eInputElements elementId, const Private::DigitalElement& element, Window* window, DAVA::int64 timestamp) const;

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();
    void OnWindowFocusChanged(DAVA::Window* window, bool focused);

private:
    InputSystem* inputSystem = nullptr;
    Private::KeyboardDeviceImpl* impl = nullptr;

    // State of each scancode key
    Array<Private::DigitalElement, static_cast<uint32>(INPUT_ELEMENTS_KB_COUNT)> keys;

    size_t endFrameConnectionToken;
    size_t primaryWindowFocusChangedToken;
};
} // namespace DAVA
