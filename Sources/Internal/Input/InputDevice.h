#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{

/**
    \defgroup input Input
    TODO: basic description of input system
*/

/**
    \ingroup input
    Enum describing possible state for digital controls.
    
	|------------------------------------------------|---------------|---------|--------------|----------|
	|                                                |   JustPressed | Pressed | JustReleased | Released |
	|------------------------------------------------|---------------|---------|--------------|----------|
	| initial control state                          | -             | -       | -            | +        |
	| right after user pressed a button (same frame) | +             | +       | -            | -        |
	| user keeps the button pressed (next frames)    | -             | +       | -            | -        |
	| user released the button (same frame)          | -             | -       | +            | +        |
	| user released the button (next frames)         | -             | -       | -            | +        |
*/
struct DigitalControlState
{
	bool IsJustPressed() { return pressed & changedThisFrame; }
	bool IsPressed() { return pressed; }
	bool IsJustReleased() { return !pressed & changedThisFrame; }
	bool IsReleased() { return !pressed; }

	bool pressed;
	bool changedThisFrame;
};

/**
    \ingroup input
    Struct describing analog control state.
    Meaning of the `x`, `y` and `z` values can be different for different devices.

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
    
    virtual ~InputDevice() {}

    /** Return unique device id */
    uint32 GetId() const;

    /**
        Get digital state for a control with specified `controlId`.
        Return value is a bitmask which can be inspected using `eDigitalControlState` enum.
        
        \pre Device should have a control with specified `controlId`

        \note Zero values stands for `RELEASE` state.
    */
    virtual DigitalControlState GetDigitalControlState(uint32 controlId) const = 0;

    /**
        Get an analog control's state with specified `controlId`

        \pre Device should have a control with specified `controlId`
    */
    virtual AnalogControlState GetAnalogControlState(uint32 controlId) const = 0;

private:
    const uint32 id;
};

inline InputDevice::InputDevice(uint32 id) : id(id)
{
}

inline uint32 InputDevice::GetId() const
{
    return id;
}

}