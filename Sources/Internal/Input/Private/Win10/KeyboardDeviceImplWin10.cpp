#include "Input/Private/Win10/KeyboardDeviceImplWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Input/Private/KeyboardDeviceImplWinCodes.h"

namespace DAVA
{
namespace Private
{
// TODO:
// UWP does not support MapVirtualKey nor does it have it's analogue.
// Even though we can obtain it using GetProcAddress func (see DllImportWin10.cpp),
// it seems like it doesn't work properly when switching input language while app is running (always maps to the language which was set during app startup).
// Until it's fixed or added, ignore any custom layouts.

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
    DVASSERT(IsKeyboardScancodeInputElement(scancodeElement));

    return static_cast<eInputElements>(scancodeElement - eInputElements::KB_COUNT_VIRTUAL);
}

eInputElements KeyboardDeviceImpl::ConvertDavaVirtualToDavaScancode(eInputElements virtualElement)
{
    DVASSERT(IsKeyboardVirtualInputElement(virtualElement));

    return static_cast<eInputElements>(virtualElement + eInputElements::KB_COUNT_VIRTUAL);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
