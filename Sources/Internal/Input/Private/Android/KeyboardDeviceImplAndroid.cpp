#include "Input/Private/Android/KeyboardDeviceImplAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
namespace Private
{
KeyboardDeviceImpl::KeyboardDeviceImpl() = default;
KeyboardDeviceImpl::~KeyboardDeviceImpl() = default;

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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
