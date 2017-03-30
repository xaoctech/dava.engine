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
        state = eDigitalElementStates::RELEASED;
    }

    void Press()
    {
        if ((state & eDigitalElementStates::PRESSED) == eDigitalElementStates::NONE)
        {
            state = eDigitalElementStates::PRESSED | eDigitalElementStates::JUST_PRESSED;
        }
    }

    void Release()
    {
        if ((state & eDigitalElementStates::PRESSED) != eDigitalElementStates::NONE)
        {
            state = eDigitalElementStates::RELEASED | eDigitalElementStates::JUST_RELEASED;
        }
    }

    void OnEndFrame()
    {
        // Clear JUST_PRESSED and JUST_RELEASED flags
        state &= ~(eDigitalElementStates::JUST_PRESSED | eDigitalElementStates::JUST_RELEASED);
    }

    eDigitalElementStates GetState() const
    {
        return state;
    }

    bool IsPressed() const
    {
        return (state & eDigitalElementStates::PRESSED) == eDigitalElementStates::PRESSED;
    }

private:
    eDigitalElementStates state;
};
}
}