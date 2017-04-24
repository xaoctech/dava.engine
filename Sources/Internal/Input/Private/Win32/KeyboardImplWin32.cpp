#include "Input/Private/Win32/KeyboardImplWin32.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Base/TemplateHelpers.h"
#include "Input/Private/KeyboardImplWinCodes.h"
#include "Utils/UTF8Utils.h"

#include <cwctype>
#include <Windows.h>

namespace DAVA
{
namespace Private
{
eInputElements KeyboardImpl::ConvertNativeScancodeToDavaScancode(uint32 nativeScancode)
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

WideString KeyboardImpl::TranslateElementToWideString(eInputElements elementId)
{
    int nativeScancode = -1;

    for (size_t i = 0; i < COUNT_OF(nativeScancodeToDavaScancode); ++i)
    {
        if (nativeScancodeToDavaScancode[i] == elementId)
        {
            nativeScancode = static_cast<int>(i);
        }
    }

    if (nativeScancode == -1)
    {
        for (size_t i = 0; i < COUNT_OF(nativeScancodeExtToDavaScancode); ++i)
        {
            if (nativeScancodeExtToDavaScancode[i] == elementId)
            {
                nativeScancode = static_cast<int>(i);
            }
        }
    }

    DVASSERT(nativeScancode >= 0);

    wchar_t character = TranslateNativeScancodeToWChar(static_cast<uint32>(nativeScancode));

    if (character == 0 || std::iswspace(character))
    {
        // Non printable
        return UTF8Utils::EncodeToWideString(GetInputElementInfo(elementId).name);
    }
    else
    {
        return WideString(1, character);
    }
}

wchar_t KeyboardImpl::TranslateNativeScancodeToWChar(uint32 nativeScancode)
{
    const uint32 nativeVirtual = MapVirtualKey(nativeScancode, MAPVK_VSC_TO_VK);
    const wchar_t character = static_cast<wchar_t>(MapVirtualKey(nativeVirtual, MAPVK_VK_TO_CHAR));

    return character;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
