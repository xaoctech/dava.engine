#pragma once

#include "Input/InputDevice.h"
#include "Input/InputEvent.h"
#include "Input/KeyboardKeys.h"

namespace DAVA
{

class DigitalControl
{
public:
	void Press()
	{
		if (state.pressed == false)
		{		
			state.pressed = true;
			state.changedThisFrame = true;
		}
	}

	void Release()
	{
		if (state.pressed == true)
		{
			state.pressed = false;
			state.changedThisFrame = true;
		}
	}

	void OnEndFrame()
	{
		state.changedThisFrame = false;
	}

	DigitalControlState const& GetState() const { return state; }

private:
	DigitalControlState state;
};

/**
    \ingroup input
    Represents keyboard input device.
*/
class KeyboardInputDevice final : public InputDevice
{
    friend class Window;

public:
    static const InputDeviceType TYPE;

    KeyboardInputDevice(uint32 id);
    ~KeyboardInputDevice();
    KeyboardInputDevice(const KeyboardInputDevice&) = delete;
    KeyboardInputDevice& operator=(const KeyboardInputDevice&) = delete;
    
    DigitalControlState GetDigitalControlState(uint32 controlId) const override;
    AnalogControlState GetAnalogControlState(uint32 controlId) const override;

private:
    void ProcessInputEvent(InputEvent& event);
    void OnEndFrame();

private:
    Array<DigitalControl, static_cast<uint32>(eKeyboardKey::TOTAL_KEYS_COUNT)> keys;
    size_t endFrameConnectionToken;
};

}