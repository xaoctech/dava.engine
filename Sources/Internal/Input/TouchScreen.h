#pragma once

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
*/
class TouchScreen final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    /**
        Return state of an i-th touch (i.e. if it's pressed, just pressed, released or just released).
        
        \note It's essentialy a shorter and more readable placeholder for `GetDigitalElementState(eInputElements:TOUCH_CLICK{i})`.
        \pre Index `i` should be in range of [0, 9].
    */
    DigitalElementState GetTouchStateByIndex(size_t i) const;

    /**
        Return position of an i-th touch. If a touch with this index is not active, zeroes are returned.

        \note It's essentialy a shorter and more readable placeholder for `GetAnalogElementState(eInputElements:TOUCH_POSITION{i})`.
        \pre Index `i` should be in range of [0, 9].
    */
    AnalogElementState GetTouchPositionByIndex(size_t i) const;

    // InputDevice overrides

    bool IsElementSupported(eInputElements elementId) const override;
    DigitalElementState GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    explicit TouchScreen(uint32 id);
    ~TouchScreen();

    TouchScreen(const TouchScreen&) = delete;
    TouchScreen& operator=(const TouchScreen&) = delete;

    bool HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e);
    void OnEndFrame();

    int GetFirstNonUsedTouchIndex() const;

private:
    InputSystem* inputSystem = nullptr;

    Array<DigitalElementState, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> clicks;
    Array<AnalogElementState, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> positions;
    Array<uint32, INPUT_ELEMENTS_TOUCH_CLICK_COUNT> nativeTouchIds;
};
} // namespace DAVA
