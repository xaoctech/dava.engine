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

    A control is a part of a device which can be used for input. For example, a keyboard button, a mouse button, a mouse wheel, gamepad's stick etc.
    There are two types of them:
        - Digital control: basically, a button, which can just be pressed and released.
        - Analog control: any control whose state can only be described using multiple float values.
                          For example, gamepad's stick position can be described using normalized x and y values.
*/

/**
    \ingroup input
	Enum describing possible states for a digital control.

	|------------------------------------------------|---------------|---------|---------------|----------|
	|                                                |  JUST_PRESSED | PRESSED | JUST_RELEASED | RELEASED |
	|------------------------------------------------|---------------|---------|---------------|----------|
	| initial control state                          | -             | -       | -             | +        |
	| right after user pressed a button (same frame) | +             | +       | -             | -        |
	| user keeps the button pressed (next frames)    | -             | +       | -             | -        |
	| user released the button (same frame)          | -             | -       | +             | +        |
	| user released the button (next frames)         | -             | -       | -             | +        |
*/
enum class eDigitalControlState : uint32
{
    NONE = 0,

    JUST_PRESSED = 1 << 0,
    PRESSED = 1 << 1,
    JUST_RELEASED = 1 << 2,
    RELEASED = 1 << 3
};

DAVA_DEFINE_ENUM_BITWISE_OPERATORS(eDigitalControlState)

/**
    \ingroup input    
	Struct describing analog control state.
    Meanings of `x`, `y` and `z` values can be different for different devices.

    For example, a gamepad's stick defines x and y values in range of [-1; 1] for according axises.
*/
struct AnalogControlState final
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

    /**
        Get digital state of a control with specified `controlId`.

        \pre Device should have a digital control with specified `controlId`.
    */
    virtual eDigitalControlState GetDigitalControlState(uint32 controlId) const = 0;

    /**
        Get an analog control's state with specified `controlId`

        \pre Device should have an analog control with specified `controlId`
    */
    virtual AnalogControlState GetAnalogControlState(uint32 controlId) const = 0;

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