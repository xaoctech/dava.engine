#include "Input/Private/Ios/KeyboardDeviceImplIos.h"

#if defined(__DAVAENGINE_IPHONE__)

namespace DAVA
{
namespace Private
{
// TODO: Implement keyboard on iOS

eInputElements KeyboardDeviceImpl::ConvertNativeScancodeToDavaScancode(uint32 nativeScancode)
{
    return eInputElements::NONE;
}

eInputElements KeyboardDeviceImpl::ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement)
{
    return eInputElements::NONE;
}

eInputElements KeyboardDeviceImpl::ConvertDavaVirtualToDavaScancode(eInputElements virtualElement)
{
    return eInputElements::NONE;
}

String KeyboardDeviceImpl::GetElementStringRepresentation(eInputElements elementId)
{
    InputElementInfo elementInfo = GetInputElementInfo(elementId);
    return elementInfo.name;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
