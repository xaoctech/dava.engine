#pragma once

#include "Engine/EngineTypes.h"
#include "Input/InputDevice.h"

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

    /** Type of the device that triggered the event */
    eInputDeviceTypes deviceType;

    /** Device that triggered the event. This value is never null */
    InputDevice* device;

    /** Id of the element which triggered the event */
    eInputElements elementId;

    /** Digital element's state. Should only be used if element with `elementId` is a digital one */
    DigitalElementState digitalState;

    /** Analog element's state. Should only be used if element with `elementId` is an analog one */
    AnalogElementState analogState;

    // Additional fields for different devices

    struct MouseEvent
    {
        bool isRelative;
    };

    struct KeyboardEvent
    {
        char32_t charCode;
        bool charRepeated;
    };

    union
    {
        MouseEvent mouseEvent;
        KeyboardEvent keyboardEvent;
    };
};
}