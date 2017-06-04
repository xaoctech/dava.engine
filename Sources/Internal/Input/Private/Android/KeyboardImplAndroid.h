#pragma once

#if defined(__DAVAENGINE_ANDROID__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class KeyboardImpl final
{
public:
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    String TranslateElementToUTF8String(eInputElements elementId);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
