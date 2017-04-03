#pragma once

#include "Input/InputElements.h"

#if defined(__DAVAENGINE_WIN_UAP__)

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
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
