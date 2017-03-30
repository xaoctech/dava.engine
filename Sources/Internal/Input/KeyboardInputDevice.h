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
*/
class KeyboardInputDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

    eInputElements ConvertScancodeToVirtual(eInputElements scancodeElement) const;
    eInputElements ConvertVirtualToScancode(eInputElements virtualElement) const;

private:
    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();

private:
    InputSystem* inputSystem = nullptr;

    Private::KeyboardDeviceImpl* impl;
    Array<Private::DigitalElement, static_cast<uint32>(eInputElements::KB_COUNT_SCANCODE)> keys;
    size_t endFrameConnectionToken;
};
}