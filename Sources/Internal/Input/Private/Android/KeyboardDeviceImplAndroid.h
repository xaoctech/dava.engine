#pragma once

#if defined(__DAVAENGINE_ANDROID__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class KeyboardDeviceImpl final
{
public:
    KeyboardDeviceImpl();
    ~KeyboardDeviceImpl();

    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    eInputElements ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement);
    eInputElements ConvertDavaVirtualToDavaScancode(eInputElements virtualElement);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
