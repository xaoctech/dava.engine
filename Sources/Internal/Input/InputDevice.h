#pragma once

#include "Base/BaseTypes.h"
#include "Input/InputElements.h"

namespace DAVA
{
/**
    \defgroup input Input

    Input system is a part of the engine which is responsible for:
        - Managing input devices (keyboards, mouses, gamepads etc.)
        - Handling input events sent by a platform
        - Storing each input device's state
*/

/**
    \ingroup input
    Enum describing possible states for a digital element.

    |                                                |  JUST_PRESSED | PRESSED | JUST_RELEASED | RELEASED |
    |------------------------------------------------|---------------|---------|---------------|----------|
    | initial element state                          | -             | -       | -             | +        |
    | right after user pressed a button (same frame) | +             | +       | -             | -        |
    | user keeps the button pressed (next frames)    | -             | +       | -             | -        |
    | user released the button (same frame)          | -             | -       | +             | +        |
    | user released the button (next frames)         | -             | -       | -             | +        |
*/
enum class eDigitalElementStates : uint32
{
    /** Helper value to use in bitwise operations */
    NONE = 0,

    /** A button has just been pressed */
    JUST_PRESSED = 1 << 0,

    /** A button is in pressed state */
    PRESSED = 1 << 1,

    /** A button has just been released */
    JUST_RELEASED = 1 << 2,

    /** A button is in a released state */
    RELEASED = 1 << 3
};

DAVA_DEFINE_ENUM_BITWISE_OPERATORS(eDigitalElementStates)

/**
    \ingroup input    
    Struct describing analog element state.
    Meanings of `x`, `y` and `z` values can be different for different elements.

    For example, a gamepad's stick defines x and y values in range of [-1; 1] for according axes.
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

/**
    \ingroup input
    Represents a virtual or a real device used for input.
    This class is responsible for:
        - Storing device's state for others to request it when needed
        - Handling native input events, transforming it to an `InputEvent` and sending it to the `InputSystem`
*/
class InputDevice
{
public:
    /** Create InputDevice instance with specified `id` */
    InputDevice(uint32 id);
    virtual ~InputDevice() = default;

    /** Return unique device id */
    uint32 GetId() const;

    /** Return true if element with specified `elementId` is supported by the device
        (i.e. it's state can be requested with either `GetDigitalElementState` or `GetAnalogElementState`)
    */
    virtual bool SupportsElement(eInputElements elementId) const = 0;

    /**
        Get state of a digital element with specified `elementId`.

        \pre Device should support specified digital element.
    */
    virtual eDigitalElementStates GetDigitalElementState(eInputElements elementId) const = 0;

    /**
        Get state of an analog element with specified `elementId`.

        \pre Device should support specified analog element.
    */
    virtual AnalogElementState GetAnalogElementState(eInputElements elementId) const = 0;

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

} // namespace DAVA
