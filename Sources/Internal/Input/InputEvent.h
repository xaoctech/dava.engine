#pragma once

#include "Input/InputDevice.h"
#include "Input/InputControls.h"

namespace DAVA
{
class Window;

/**
    \ingroup input
    Represents input event triggered by some device.
*/
struct InputEvent
{
    /** The window this event is addressed to */
    Window* window;

    /** Event timestamp */
    float64 timestamp;

    /** Type of the device */
    InputDeviceType deviceType;

    /** Id of the device */
    uint32 deviceId;

    /**
        Device's control Id.
        Control is a part of a device that can be used for input (like a button, a stick, a wheel).
    */
    eInputControl controlId;

    union
    {
        /** Digital control's state */
        eDigitalControlState digitalState;

        /** Analog control's state */
        AnalogControlState analogState;
    };

    // Additional fields for different devices

    struct KeyboardEvent
    {
        bool isCharEvent;
    };

    union
    {
        KeyboardEvent keyboardEvent;
    };
};
}