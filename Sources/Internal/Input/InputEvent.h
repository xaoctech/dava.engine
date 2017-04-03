#pragma once

#include "Engine/EngineTypes.h"
#include "Input/InputDevice.h"
#include "Input/InputElements.h"

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
    eInputDeviceTypes deviceType;

    /** Id of the device */
    uint32 deviceId;

    /** Id of the element which triggered the event */
    eInputElements elementId;

    union
    {
        /** Digital element's state */
        eDigitalElementStates digitalState;

        /** Analog element's state */
        AnalogElementState analogState;
    };

    // Additional fields for different devices

    struct MouseEvent
    {
        bool isRelative;
    };

    union
    {
        MouseEvent mouseEvent;
    };
};
}