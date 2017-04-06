#include "Input/Private/Win10/KeyboardDeviceImplWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
