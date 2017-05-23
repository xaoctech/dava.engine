#pragma once

#if defined(__DAVAENGINE_MACOS__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class KeyboardImpl final
{
public:
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    uint32 ConvertDavaScancodeToNativeScancode(eInputElements nativeScancode);
    WideString TranslateElementToWideString(eInputElements elementId);
};

} // namespace Private
} // namespace DAVA

#endif
