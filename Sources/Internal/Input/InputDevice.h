#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \defgroup input Input

    Input system is a part of the engine which is responsible for:
        - Managing input devices (keyboards, mouses, gamepads etc.)
        - Handling input events sent by a platform
        - Storing each input device's state

    An input element is a part of a device which can be used for input. For example, a keyboard button, a mouse button, a mouse wheel, gamepad's stick etc.
    There are two types of them:
        - Digital element: basically, a button, which can just be pressed and released.
        - Analog element: any element whose state can only be described using multiple float values.
                          For example, gamepad's stick position can be described using normalized x and y values.
*/

/**
    \ingroup input
    Enum describing possible states for a digital element.

    |------------------------------------------------|---------------|---------|---------------|----------|
    |                                                |  JUST_PRESSED | PRESSED | JUST_RELEASED | RELEASED |
    |------------------------------------------------|---------------|---------|---------------|----------|
    | initial element state                          | -             | -       | -             | +        |
    | right after user pressed a button (same frame) | +             | +       | -             | -        |
    | user keeps the button pressed (next frames)    | -             | +       | -             | -        |
    | user released the button (same frame)          | -             | -       | +             | +        |
    | user released the button (next frames)         | -             | -       | -             | +        |
*/
enum class eDigitalElementState : uint32
{
    // Helper value to use in bitwise operations
    NONE = 0,

    JUST_PRESSED = 1 << 0,
    PRESSED = 1 << 1,
    JUST_RELEASED = 1 << 2,
    RELEASED = 1 << 3
};

DAVA_DEFINE_ENUM_BITWISE_OPERATORS(eDigitalElementState)

/**
    \ingroup input    
	Struct describing analog element state.
    Meanings of `x`, `y` and `z` values can be different for different devices.

    For example, a gamepad's stick defines x and y values in range of [-1; 1] for according axises.
*/
struct AnalogElementState final
{
    /** Analog X value */
    float32 x;

    /** Analog Y value */
    float32 y;

    /** Analog Z value */
    float32 z;
};

// TODO: decide which type is better
using InputDeviceType = uint32;

/**
    \ingroup input
    Represents a virtual or a real device used for input.
    This class is responsible for storing device's state for others to request it when needed.
*/
class InputDevice
{
public:
    /** Create InputDevice instance with specified `id` */
    InputDevice(uint32 id);

    virtual ~InputDevice()
    {
    }

    /** Return unique device id */
    uint32 GetId() const;

    virtual bool SupportsElement(uint32 elementId) const = 0;

    /**
        Get digital state of an element with specified `elementId`.

        \pre Device should have a digital element with specified id. It can be checked with `SupportsElement` method.
    */
    virtual eDigitalElementState GetDigitalElementState(uint32 elementId) const = 0;

    /**
        Get an analog element's state with specified `elementId`

        \pre Device should have an analog element with specified id.
    */
    virtual AnalogElementState GetAnalogElementState(uint32 elementId) const = 0;

private:
    const uint32 id;
};

inline InputDevice::InputDevice(uint32 id)
    : id(id)
{
}

inline uint32 InputDevice::GetId() const
{
    return id;
}
}