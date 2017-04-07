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
    Input device that represents touch screen.
    It manages all touch input elements (`eInputElements::TOUCH_*`), where TOUCH0 corresponds to the first touch, TOUCH1 corresponds to the second touch etc.

    This device separates touches into two types of elements:
        - Click elements (`TOUCH_CLICK0`, `TOUCH_CLICK1` etc.) represent digital part (pressed or released states)
        - Position elements (`TOUCH_POSITION0`, `TOUCH_POSITION1` etc.) represent analog part (position on a screen)
*/
class TouchDevice final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    // InputDevice overrides
    bool SupportsElement(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    TouchDevice(uint32 id);
    ~TouchDevice();
    TouchDevice(const TouchDevice&) = delete;
    TouchDevice& operator=(const TouchDevice&) = delete;

    bool HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();

    int GetFirstNonUsedTouchIndex() const;

private:
    InputSystem* inputSystem = nullptr;

    Array<eDigitalElementStates, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> clicks;
    Array<AnalogElementState, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> positions;
    Array<uint32, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> nativeTouchIds;
};
} // namespace DAVA
