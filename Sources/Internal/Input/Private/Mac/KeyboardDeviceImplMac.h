#pragma once

#if defined(__DAVAENGINE_MACOS__)

#include "Input/InputElements.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

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
}
}

#endif
