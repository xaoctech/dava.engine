#include "Input/Private/Ios/KeyboardImplIos.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Utils/UTF8Utils.h"

namespace DAVA
{
namespace Private
{
// TODO: Implement keyboard on iOS

eInputElements KeyboardImpl::ConvertNativeScancodeToDavaScancode(uint32 nativeScancode)
{
    return eInputElements::NONE;
}

uint32 KeyboardImpl::ConvertDavaScancodeToNativeScancode(eInputElements nativeScancode)
{
    return 0;
}

WideString KeyboardImpl::TranslateElementToWideString(eInputElements elementId)
{
    return UTF8Utils::EncodeToWideString(GetInputElementInfo(elementId).name);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
