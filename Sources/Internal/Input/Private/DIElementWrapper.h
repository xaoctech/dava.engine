#pragma once

#include "Input/InputDevice.h"

namespace DAVA
{
// Wrapper over eDigitalElementStates enum
struct DIElementWrapper
{
    DIElementWrapper(eDigitalElementStates& stateRef);

    void Press();
    void Release();
    eDigitalElementStates GetState() const;
    bool IsPressed() const;
    void OnEndFrame();

    eDigitalElementStates& state;
};

inline DIElementWrapper::DIElementWrapper(eDigitalElementStates& stateRef)
    : state(stateRef)
{
}

inline void DIElementWrapper::Press()
{
    if ((state & eDigitalElementStates::PRESSED) == eDigitalElementStates::NONE)
    {
        state = eDigitalElementStates::PRESSED | eDigitalElementStates::JUST_PRESSED;
    }
}

inline void DIElementWrapper::Release()
{
    if ((state & eDigitalElementStates::PRESSED) != eDigitalElementStates::NONE)
    {
        state = eDigitalElementStates::RELEASED | eDigitalElementStates::JUST_RELEASED;
    }
}

inline eDigitalElementStates DIElementWrapper::GetState() const
{
    return state;
}

inline bool DIElementWrapper::IsPressed() const
{
    return (state & eDigitalElementStates::PRESSED) == eDigitalElementStates::PRESSED;
}

inline void DIElementWrapper::OnEndFrame()
{
    // Clear JUST_PRESSED and JUST_RELEASED flags
    state &= ~(eDigitalElementStates::JUST_PRESSED | eDigitalElementStates::JUST_RELEASED);
}

} // namespace DAVA
