#pragma once

#include "Input/Private/DigitalElement.h"
#include "Input/InputElements.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

namespace DAVA
{
class InputSystem;

/**
     \ingroup input
     Represents touch input device.
 */
class TouchDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    TouchDevice(uint32 id);
    ~TouchDevice();
    TouchDevice(const TouchDevice&) = delete;
    TouchDevice& operator=(const TouchDevice&) = delete;

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();

    int GetFirstNonUsedTouchIndex() const;

private:
    InputSystem* inputSystem = nullptr;
    size_t endFrameConnectionToken;

    Array<Private::DigitalElement, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> clicks;
    Array<AnalogElementState, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> positions;
    Array<uint32, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> nativeTouchIds;
};
} // namespace DAVA
