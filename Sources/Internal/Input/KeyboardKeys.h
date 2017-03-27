#pragma once

#include "Base/BaseTypes.h"
#include "Input/InputControls.h"

namespace DAVA
{
/**
    \ingroup input
    Translates platform-dependent key code to `eKeyboardKey` value.
*/
eInputControl SystemKeyToDavaKey(uint32 systemKeyCode);
}