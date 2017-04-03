#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Input/Private/KeyboardDeviceImplWinCodes.h"

namespace DAVA
{
namespace Private
{
eInputElements KeyboardDeviceImpl::ConvertNativeScancodeToDavaScancode(uint32 nativeScancode)
{
    const bool isExtended = (nativeScancode & 0xE000) == 0xE000;
    const uint32 nonExtendedScancode = nativeScancode & 0x00FF;

    if (isExtended)
    {
        return nativeScancodeExtToDavaScancode[nonExtendedScancode];
    }
    else
    {
        return nativeScancodeToDavaScancode[nonExtendedScancode];
    }
}

eInputElements KeyboardDeviceImpl::ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement)
{
    DVASSERT(scancodeElement >= eInputElements::KB_FIRST_SCANCODE && scancodeElement <= eInputElements::KB_LAST_SCANCODE);

    // UWP does not support MapVirtualKey nor does it have it's analogue
    // Until it's added, ignore any custom layouts
    return static_cast<eInputElements>(scancodeElement - eInputElements::KB_COUNT_VIRTUAL);
}

eInputElements KeyboardDeviceImpl::ConvertDavaVirtualToDavaScancode(eInputElements virtualElement)
{
    DVASSERT(virtualElement >= eInputElements::KB_FIRST_VIRTUAL && virtualElement <= eInputElements::KB_LAST_VIRTUAL);

    // UWP does not support MapVirtualKey nor does it have it's analogue
    // Until it's added, ignore any custom layouts
    return static_cast<eInputElements>(virtualElement + eInputElements::KB_COUNT_VIRTUAL);
}

} // namespace Private
} // namespace DAVA

#endif
