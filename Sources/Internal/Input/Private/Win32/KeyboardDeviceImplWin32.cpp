#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Base/TemplateHelpers.h"
#include "Input/Private/KeyboardDeviceImplWinCodes.h"
#include "Utils/UTF8Utils.h"

#include <Windows.h>

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

WideString KeyboardDeviceImpl::TranslateElementToWideString(eInputElements elementId)
{
	for (size_t i = 0; i < COUNT_OF(nativeScancodeToDavaScancode); ++i)
	{
		if (nativeScancodeToDavaScancode[i] == elementId)
		{
			const uint32 nativeVirtual = MapVirtualKey(i, MAPVK_VSC_TO_VK);
			const wchar_t character = static_cast<wchar_t>(MapVirtualKey(nativeVirtual, MAPVK_VK_TO_CHAR));
			
			if (character == 0)
			{
				// Non printable
				return UTF8Utils::EncodeToWideString(GetInputElementInfo(elementId).name);
			}
			else
			{
				return WideString(1, character);
			}
		}
	}

	DVASSERT(false);
	return WideString(L"");
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
