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
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    eInputElements ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement);
    eInputElements ConvertDavaVirtualToDavaScancode(eInputElements virtualElement);

    String GetElementStringRepresentation(eInputElements elementId);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
