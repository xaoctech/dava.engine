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

/**
    \addtogroup input
\{
*/

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

	Usage example:
	\code
	GetEngineContext()->inputListener->Listen(eInputListenerModes::DIGITAL_SINGLE_WITH_MODIFIERS, MakeFunction(OnInputListeningEnded));

	void OnInputListeningEnded(bool cancelled, DAVA::Vector<DAVA::InputEvent> input)
	{
		if (!cancelled)
		{
			// Handle captured input,
			// e.g. bind it to the action system, print it, etc.
		}
	}
	\endcode
*/
class InputListener final
{
    friend class Private::EngineBackend;

public:
    /** Listen for input in specified `mode`, across all devices. */
    void Listen(eInputListenerModes mode, Function<void(bool, Vector<InputEvent>)> callback);

    /** Listen input in specified `mode`, devices other than the one with `deviceId` are ignored. */
    void Listen(eInputListenerModes mode, Function<void(bool, Vector<InputEvent>)> callback, uint32 deviceId);

    /** Listen input in specified `mode`, devices with type different from `deviceTypesMask` are ignored. */
    void Listen(eInputListenerModes mode, Function<void(bool, Vector<InputEvent>)> callback, eInputDeviceTypes deviceTypesMask);

    /** Return true if listening is active. */
    bool IsListening() const;

    /** Stop listening for input events. Callback, provided to `Listen` function, will not be called. */
    void StopListening();

private:
    InputListener() = default;
    InputListener(const InputListener&) = delete;
    InputListener& operator=(const InputListener&) = delete;

    void Listen(eInputListenerModes mode, Function<void(bool, Vector<InputEvent>)> callback, uint32 deviceId, eInputDeviceTypes deviceTypesMask);

    bool OnInputEvent(const InputEvent& e);

private:
    uint32 inputHandlerToken = 0;

    eInputListenerModes currentMode;
    Function<void(bool, Vector<InputEvent>)> currentCallback;
    uint32 currentDeviceId;
    eInputDeviceTypes currentDeviceTypesMask;

    Vector<InputEvent> result;
};

/**
    \}
*/

} // namespace DAVA
