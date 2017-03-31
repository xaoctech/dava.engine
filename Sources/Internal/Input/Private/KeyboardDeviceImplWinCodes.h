#pragma once

// Mappings from native to dava scancodes are the same for both Win32 and UWP, so put them in common header

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
extern const eInputElements nativeScancodeToDavaScancode[0x59];
extern const eInputElements nativeScancodeExtToDavaScancode[0x5E];
}
}