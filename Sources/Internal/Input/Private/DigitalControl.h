#pragma once

#include "Input/InputDevice.h"

namespace DAVA
{
namespace Private
{

// Helper class to use in input devices that have digital controls
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

}
}