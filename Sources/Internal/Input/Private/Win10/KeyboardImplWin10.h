#pragma once

#include "Input/InputElements.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
namespace Private
{
class KeyboardImpl final
{
public:
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    uint32 ConvertDavaScancodeToNativeScancode(eInputElements nativeScancode);
    String TranslateElementToUTF8String(eInputElements elementId);

private:
    wchar_t TranslateNativeScancodeToWChar(uint32 nativeScancode);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
