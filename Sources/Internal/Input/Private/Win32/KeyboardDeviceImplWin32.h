#pragma once

#if defined(__DAVAENGINE_WIN32__)

#include "Input/InputElements.h"

namespace DAVA
{
namespace Private
{
struct MainDispatcherEvent;
class KeyboardDeviceImpl final
{
public:
    KeyboardDeviceImpl();
    ~KeyboardDeviceImpl();

    eInputElements ConvertNativeScancodeToDavaScancode(uint32 nativeScancode);
    eInputElements ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement);
    eInputElements ConvertDavaVirtualToDavaScancode(eInputElements virtualElement);

private:
    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void UpdateVirtualToScancodeMap();
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
