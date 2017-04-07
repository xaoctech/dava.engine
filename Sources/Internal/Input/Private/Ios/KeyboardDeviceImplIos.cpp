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

WideString KeyboardDeviceImpl::TranslateElementToWideString(eInputElements elementId)
{
    return UTF8Utils::EncodeToWideString(GetInputElementInfo(elementId).name);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
