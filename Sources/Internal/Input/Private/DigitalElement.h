#pragma once

#include "Input/InputDevice.h"

namespace DAVA
{
namespace Private
{
// Helper class to use in input devices that have digital elements
class DigitalElement
{
public:
    DigitalElement()
    {
        state = eDigitalElementState::RELEASED;
    }

    void Press()
    {
        if ((state & eDigitalElementState::PRESSED) == eDigitalElementState::NONE)
        {
            state = eDigitalElementState::PRESSED | eDigitalElementState::JUST_PRESSED;
        }
    }

    void Release()
    {
        if ((state & eDigitalElementState::PRESSED) != eDigitalElementState::NONE)
        {
            state = eDigitalElementState::RELEASED | eDigitalElementState::JUST_RELEASED;
        }
    }

    void OnEndFrame()
    {
        // Clear JUST_PRESSED and JUST_RELEASED flags
        state &= ~(eDigitalElementState::JUST_PRESSED | eDigitalElementState::JUST_RELEASED);
    }

    eDigitalElementState GetState() const
    {
        return state;
    }

    bool IsPressed() const
    {
        return (state & eDigitalElementState::PRESSED) == eDigitalElementState::PRESSED;
    }

private:
    eDigitalElementState state;
};
}
}