#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Input/InputEvent.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
}

/** Describes input listening modes */
enum class eInputListenerModes
{
    /**
		Listen for a single element which is not a keyboard modifier.
		Examples: W, left mouse button, etc.
	*/
    DIGITAL_SINGLE_WITHOUT_MODIFIERS,

    /**
		Listen for a single element which is not a keyboard modifier with possible modifiers.
		Examples: W, Shift + W, Ctrl + right mouse button, etc.
	*/
    DIGITAL_SINGLE_WITH_MODIFIERS,

    /**
		Listen for sequence of any multiple digital elements.
	*/
    DIGITAL_MULTIPLE_ANY,

    /**
		Listen for a single analog element input.
		Examples: mouse move, gamepad's stick.
	*/
    ANALOG,
};

/**
	Helper class which is able to capture and report user input.
	Can be helpful for control settings UI.
*/
class InputListener final
{
    friend class Private::EngineBackend;

public:
    /** Listen for input in specified `mode`, across all devices */
    void Listen(eInputListenerModes mode, Function<void(Vector<eInputElements>)> callback);

    /** Listen input in specified `mode`, devices other than the one with `deviceId` are ignored */
    void Listen(eInputListenerModes mode, Function<void(Vector<eInputElements>)> callback, uint32 deviceId);

    /** Listen input in specified `mode`, devices with type different from `deviceTypesMask` are ignored */
    void Listen(eInputListenerModes mode, Function<void(Vector<eInputElements>)> callback, eInputDeviceTypes deviceTypesMask);

    /** Return true if listening is active. */
    bool IsListening() const;

    /** Stop listening for input events. Callback, provided to `Listen` function, will not be called. */
    void StopListening();

private:
    InputListener()
    {
    }
    InputListener(const InputListener&) = delete;
    InputListener& operator=(const InputListener&) = delete;

    void Listen(eInputListenerModes mode, Function<void(Vector<eInputElements>)> callback, uint32 deviceId, eInputDeviceTypes deviceTypesMask);

    bool OnInputEvent(const InputEvent& e);

private:
    uint32 inputHandlerToken = 0;

    eInputListenerModes currentMode;
    Function<void(Vector<eInputElements>)> currentCallback;
    uint32 currentDeviceId;
    eInputDeviceTypes currentDeviceTypesMask;

    Vector<eInputElements> result;
};
}