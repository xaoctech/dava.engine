#pragma once

#include "Input/InputSystemTypes.h"
#include "Input/InputElements.h"

namespace DAVA
{
/**
    \ingroup input

    Represents a virtual or a real device used for input.
    This class is responsible for:
        - Storing device's state for others to request it when needed
        - Handling native input events, transforming it to an `InputEvent` and sending it to the `InputSystem`

    Device instances can be obtained using `DeviceManager` methods: `GetKeyboard`, `GetMouse`, `GetTouchScreen`, and `GetGamepad`.

    Derived classes are welcome to introduce helper method for easier and more readable access to its elements,
    e.g. a mouse can provide GetPosition() method which can be a wrapper around GetAnalogElementState(eInputElements::MOUSE_POSITION), etc.
*/
class InputDevice
{
public:
    /** Create InputDevice instance with specified `id` */
    explicit InputDevice(uint32 id);

    virtual ~InputDevice() = default;

    /** Return unique device id */
    uint32 GetId() const;

    /** Return `true` if an element with specified `elementId` is supported by the device
        (i.e. its state can be requested with either `GetDigitalElementState` or `GetAnalogElementState`).
    */
    virtual bool IsElementSupported(eInputElements elementId) const = 0;

    /**
        Get state of a digital element with specified `elementId`.

        \pre Device should support specified digital element.
    */
    virtual DigitalElementState GetDigitalElementState(eInputElements elementId) const = 0;

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
