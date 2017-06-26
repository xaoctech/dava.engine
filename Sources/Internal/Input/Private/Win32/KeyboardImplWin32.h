#pragma once

#if defined(__DAVAENGINE_WIN32__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class KeyboardImpl final
{
public:
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode, uint32 nativeVirtual);
    uint32 ConvertDavaScancodeToNativeScancode(eInputElements nativeScancode);
    String TranslateElementToUTF8String(eInputElements elementId);

private:
    wchar_t TranslateNativeScancodeToWChar(uint32 nativeScancode);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
