#pragma once

#if defined(__DAVAENGINE_MACOS__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
class KeyboardDeviceImpl final
{
public:
    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);

private:
    void UpdateVirtualToScancodeMap();
};

} // namespace Private
} // namespace DAVA

#endif
