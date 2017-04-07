#pragma once

#include "Base/Token.h"
#include "Input/InputDevice.h"

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

    /**
        Translate keyboard key into wide string, using curring keyboard layout.
    */
    WideString TranslateElementToWideString(eInputElements elementId) const;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    void OnEndFrame();
    void OnWindowFocusChanged(DAVA::Window* window, bool focused);

    bool HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e);
    void CreateAndSendInputEvent(eInputElements elementId, eDigitalElementStates element, Window* window, int64 timestamp) const;

private:
    InputSystem* inputSystem = nullptr;
    Private::KeyboardDeviceImpl* impl = nullptr;

    // State of each physical key
    Array<eDigitalElementStates, INPUT_ELEMENTS_KB_COUNT> keys;

    Token endFrameConnectionToken;
    Token primaryWindowFocusChangedToken;
};
} // namespace DAVA
