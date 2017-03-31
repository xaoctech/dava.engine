#pragma once

#include "Input/Private/DigitalElement.h"
#include "Input/InputElements.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

namespace DAVA
{
class InputSystem;

namespace Private
{
class KeyboardDeviceImpl;
}

/**
    \ingroup input
    Represents keyboard input device.

	This class is responsible for handling all keyboard input elements, both virtual and scancodes.
*/
class KeyboardInputDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

    /** Convert a scancode key to a virtual key according to current keyboard layout */
    eInputElements ConvertScancodeToVirtual(eInputElements scancodeElement) const;

    /** Convert a virtual key to a scancode key according to current keyboard layout */
    eInputElements ConvertVirtualToScancode(eInputElements virtualElement) const;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    void CreateAndSendInputEvent(eInputElements scancodeElementId, const Private::DigitalElement& element, Window* window, DAVA::int64 timestamp);

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();
    void OnWindowFocusChanged(DAVA::Window* window, bool focused);

private:
    InputSystem* inputSystem = nullptr;
    Private::KeyboardDeviceImpl* impl = nullptr;
    Array<Private::DigitalElement, static_cast<uint32>(eInputElements::KB_COUNT_SCANCODE)> keys;
    size_t endFrameConnectionToken;
    size_t primaryWindowFocusChangedToken;
};
}