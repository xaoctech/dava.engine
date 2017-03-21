#pragma once

#include "Input/InputEvent.h"
#include "Functional/Signal.h"
#include "Base/FastName.h"

namespace DAVA
{
/**
    \defgroup actions Actions
    // TODO: description
*/

/**
    \ingroup actions
    Describes an action triggered by `ActionSystem`.
*/
struct Action
{
    /** Id of the action */
    FastName actionId;

    /** Id of the device which triggered the action */
    uint32 triggeredDeviceId;

    /** Type of the device that triggered the action */
    uint32 triggeredDeviceType;

    /** State of the analog control that triggered the action. This field should not be used if it was a pure digital action */
    AnalogControlState analogControlState;
};

/**
    \ingroup actions
    Class that handles actions bindings and notifies when an action is triggered
*/
class ActionSystem final
{
public:
    /** Struct describing specific control's state, used for binding */
    struct DeviceDigitalControlState
    {
        uint32 deviceId;
        uint32 controlId;
        eDigitalControlState stateMask;

        DeviceDigitalControlState()
        {
            // To indicate empty state
            deviceId = -1;
        }

        DeviceDigitalControlState(uint32 deviceId, uint32 controlId, eDigitalControlState stateMask)
            : deviceId(deviceId)
            , controlId(controlId)
            , stateMask(stateMask)
        {
        }
    };

    ActionSystem();
    ~ActionSystem();
    ActionSystem(const ActionSystem&) = delete;
    ActionSystem& operator=(const ActionSystem&) = delete;

    /** Bind an action with specified `actionId` to a digital control */
    void BindDigitalAction(FastName actionId, DeviceDigitalControlState state);

    /** Bind an action with specified `actionId` to digital controls */
    void BindDigitalAction(FastName actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2);

    /** Bind an action with specified `actionId` to digital controls */
    void BindDigitalAction(FastName actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2, DeviceDigitalControlState state3);

    /** Emits when an action happens */
    Signal<Action> ActionTriggered;

private:
    struct DigitalActionBinding
    {
        FastName actionId;
        DeviceDigitalControlState states[3];
    };

    bool OnInputEvent(const InputEvent& event);

    Vector<DigitalActionBinding> digitalBindings;

    SigConnectionID inputSystemHandlerToken;
};
}