#include "Input/Private/Win10/KeyboardImplWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Input/Private/KeyboardImplWinCodes.h"
#include "Base/TemplateHelpers.h"
#include "Utils/UTF8Utils.h"
#include "Engine/Private/UWP/DllImportWin10.h"

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

uint32 KeyboardImpl::ConvertDavaScancodeToNativeScancode(eInputElements elementId)
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
                nativeScancode = static_cast<int>(i) | 0xE000;
            }
        }
    }

    DVASSERT(nativeScancode >= 0);

    return static_cast<uint32>(nativeScancode);
}

String KeyboardImpl::TranslateElementToUTF8String(eInputElements elementId)
{
    if (DllImport::fnMapVirtualKey == nullptr)
    {
        // Return default en name if MapVirtualKey couldn't be imported
        return GetInputElementInfo(elementId).name;
    }

    int nativeScancode = ConvertDavaScancodeToNativeScancode(elementId);
    wchar_t character = TranslateNativeScancodeToWChar(static_cast<uint32>(nativeScancode));

    if (character == 0)
    {
        // Non printable
        return GetInputElementInfo(elementId).name;
    }
    else
    {
        return UTF8Utils::EncodeToUTF8(WideString(1, character));
    }
}

wchar_t KeyboardImpl::TranslateNativeScancodeToWChar(uint32 nativeScancode)
{
    const uint32 nativeVirtual = DllImport::fnMapVirtualKey(nativeScancode, MAPVK_VSC_TO_VK);
    const wchar_t character = static_cast<wchar_t>(DllImport::fnMapVirtualKey(nativeVirtual, MAPVK_VK_TO_CHAR));

    return character;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
