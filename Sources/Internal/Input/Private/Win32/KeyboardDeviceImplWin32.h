#pragma once

#include "Input/InputElements.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

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

private:
    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void UpdateVirtualToScancodeMap();
};
}
}