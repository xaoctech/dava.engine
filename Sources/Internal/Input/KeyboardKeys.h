#pragma once

#include "Base/BaseTypes.h"
#include "Input/InputElements.h"

namespace DAVA
{
/**
    \ingroup input
    Translates platform-dependent key code to `eKeyboardKey` value.
*/
eInputElements SystemKeyToDavaKey(uint32 systemKeyCode);
}