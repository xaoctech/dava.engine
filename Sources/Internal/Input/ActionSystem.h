#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Functional/Signal.h"
#include "Input/InputDevice.h"
#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
class ActionSystemImpl;
}

/**
\defgroup actions Actions
*/

/**
    \ingroup actions
    Describes an action triggered by `ActionSystem`.
*/
struct Action final
{
    /** Id of the action */
    FastName actionId;

    /** Id of the device whose event triggered the action. This value is never null */
    InputDevice* triggeredDevice;

    /**
        If the action was triggered using `AnalogBinding`, this field contains state of the element which triggered the action.
        If the action was triggered using `DigitalBinding`, this field contains value taken from `DigitalBinding::outputAnalogState` field (user-specified).
   */
    AnalogElementState analogState;
};

// Forward declare these two since they use MAX_DIGITAL_STATES_COUNT from ActionSystem class
struct DigitalBinding;
struct AnalogBinding;

/**
    \ingroup actions
    Specified list of bindings for action system.
*/
struct ActionSet final
{
    /** Name of the set */
    String name;

    /** List of bindings to digital elements. */
    Vector<DigitalBinding> digitalBindings;

    /** List of bindings to analog elements. */
    Vector<AnalogBinding> analogBindings;
};

/**
    \ingroup actions
    Class which is responsible for binding action sets and notifying when action is triggered.
    `BindSet` function is used to bind an action set.
    `ActionTriggered` is emitted when an action is triggered.

    Usage example:
    \code
    void OnEnterBattle()
    {
        // Create and fill action set

        ActionSet set;
        
        // Fire with a gamepad's X button
        DigitalBinding fireBinding;
        fireBinding.actionId = FIRE;
        fireBinding.digitalElements[0] = eInputElements::GAMEPAD_X;
        fireBinding.digitalStates[0] = DigitalElementState(eDigitalElementStates::PRESSED);
        set.digitalBindings.push_back(fireBinding);
        
        // Move with left thumb
        AnalogBinding moveBinding;
        moveBinding.actionId = MOVE;
        moveBinding.analogElementId = eInputElements::GAMEPAD_AXIS_LTHUMB;
        set.analogBindings.push_back(moveBinding);

        ActionSystem* actionSystem = GetEngineContext()->actionSystem;

        actionSystem->BindSet(set);
        actionSystem->ActionTriggered.Connect(&OnAction);
    }

    void OnAction(Action action)
    {
        if (action.actionId == FIRE)
        {
            // Fire a bullet
        }
        else if (action.actionId == MOVE)
        {
            Vector2 velocity(action.analogState.x, action.analogState.y);
            
            // Move a character
        }
    }

    \endcode
*/
class ActionSystem final
{
    friend class Private::EngineBackend; // For creation

public:
    /**
        Binds an action set to a specific device. If there's already a set binded to the same device, it will be replaced by the new one.
        All binding requirements will be checked only on this device.
        Useful for local multiplayer setup, since we can bind one set to a gamepad-1 and another set to a gamepad-2.
    */
    void BindSet(const ActionSet& actionSet, uint32 deviceId);

    /**
        Binds an action set to two devices. If there's already a set binded to a device, it will be replaced by the new one.
        All binding requirements will be checked only on these devices.
        Can be used to bind a set to a keyboard and mouse in local multiplayer setup.
    */
    void BindSet(const ActionSet& actionSet, uint32 deviceId1, uint32 deviceId2);

    /** Unbind all the sets. */
    void UnbindAllSets();

public:
    /** Emits when an action is triggered */
    Signal<Action> ActionTriggered;

public:
    // Maximum number of digital elements states which is supported by action system
    // I.e. user can only specify up to MAX_DIGITAL_STATES_COUNT buttons to trigger an action
    static const size_t MAX_DIGITAL_STATES_COUNT = 3;

private:
    ActionSystem();
    ~ActionSystem();
    ActionSystem(const ActionSystem&) = delete;
    ActionSystem& operator=(const ActionSystem&) = delete;

private:
    Private::ActionSystemImpl* impl = nullptr;
};

/**
    \ingroup actions
    Describes an action bound to a number of digital elements.
*/
struct DigitalBinding final
{
    DigitalBinding()
    {
        std::fill(digitalElements.begin(), digitalElements.end(), eInputElements::NONE);
    }

    /** Id of the action to trigger */
    FastName actionId;

    /**
        Array of digital elements whose specific states (described in `digitalStates` fields) are required for the action to be triggered.
        digitalElements[i]'s required state is digitalStates[i].
    */
    Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT> digitalElements;

    /**
        Array of digital element states which are required for the action to be triggered.
        digitalStates[i] is required state for digitalElements[i].
    */
    Array<DigitalElementState, ActionSystem::MAX_DIGITAL_STATES_COUNT> digitalStates;

    /**
        Analog state which will be put into a triggered action.
        Useful in cases when a button performs the same action as an analog element.
        For example if a user uses gamepad's stick for MOVE action (which sends [-1; 1] values for x and y axes), he might also want to bind movement to WASD buttons.
        In this situation, he can specify (0, 1) for W, (-1, 0) for A, (0, -1) for S and (0, 1) for D, thus unifiying input handling code for this action:

        \code
        if (action.actionId == MOVE)
        {
            // Don't care about the way analogState was filled
            // Just know that it's in a range [-1; 1] for x and y
            Vector2 velocity(action.analogState.x, action.analogState.y);
            Move(velocity);
        }
        \endcode
    */
    AnalogElementState outputAnalogState;
};

/**
    \ingroup actions
    Describes an action bound to an analog element.

    Additional digital modifiers can be specified.
    For example, we can bind an action to shift (digital modifier) + mouse move (analog element).
*/
struct AnalogBinding final
{
    AnalogBinding()
    {
        std::fill(digitalElements.begin(), digitalElements.end(), eInputElements::NONE);
    }

    /** Id of the action to trigger. */
    FastName actionId;

    /** Id of the analog element whose state changes will trigger the action. */
    eInputElements analogElementId;

    /**
        Additional digital elements whose specific states (described in `digitalStates` fields) are required for the action to be triggered.
        digitalElements[i]'s required state is digitalStates[i].
    */
    Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT> digitalElements;

    /**
        Array of digital element states which are required for the action to be triggered.
        digitalStates[i] is required state for digitalElements[i].
    */
    Array<DigitalElementState, ActionSystem::MAX_DIGITAL_STATES_COUNT> digitalStates;
};
}
