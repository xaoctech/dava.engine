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
    DigitalControl()
    {
        state = eDigitalControlState::RELEASED;
    }

    void Press()
    {
        if ((state & eDigitalControlState::PRESSED) == eDigitalControlState::NONE)
        {
            state = eDigitalControlState::PRESSED | eDigitalControlState::JUST_PRESSED;
        }
    }

    void Release()
    {
        if ((state & eDigitalControlState::PRESSED) != eDigitalControlState::NONE)
        {
            state = eDigitalControlState::RELEASED | eDigitalControlState::JUST_RELEASED;
        }
    }

    void OnEndFrame()
    {
        // Clear JUST_PRESSED and JUST_RELEASED flags
        state &= ~(eDigitalControlState::JUST_PRESSED | eDigitalControlState::JUST_RELEASED);
    }

    eDigitalControlState GetState() const
    {
        return state;
    }

private:
    eDigitalControlState state;
};
}
}