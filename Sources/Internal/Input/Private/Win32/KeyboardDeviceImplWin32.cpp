#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/Private/KeyboardDeviceImplWinCodes.h"

namespace DAVA
{
namespace Private
{
// Maps virtual key to win32 native key
// 0x00 means that key is not mappable (i.e. it is the same on all input layouts)
static const uint32 davaVirtualMappableToNativeVirtual[] =
{
  0x00, // KB_1_VIRTUAL
  0x00, // KB_2_VIRTUAL
  0x00, // KB_3_VIRTUAL
  0x00, // KB_4_VIRTUAL
  0x00, // KB_5_VIRTUAL
  0x00, // KB_6_VIRTUAL
  0x00, // KB_7_VIRTUAL
  0x00, // KB_8_VIRTUAL
  0x00, // KB_9_VIRTUAL
  0x00, // KB_0_VIRTUAL
  0x41, // KB_A_VIRTUAL
  0x42, // KB_B_VIRTUAL
  0x43, // KB_C_VIRTUAL
  0x44, // KB_D_VIRTUAL
  0x45, // KB_E_VIRTUAL
  0x46, // KB_F_VIRTUAL
  0x47, // KB_G_VIRTUAL
  0x48, // KB_H_VIRTUAL
  0x49, // KB_I_VIRTUAL
  0x4A, // KB_J_VIRTUAL
  0x4B, // KB_K_VIRTUAL
  0x4C, // KB_L_VIRTUAL
  0x4D, // KB_M_VIRTUAL
  0x4E, // KB_N_VIRTUAL
  0x4F, // KB_O_VIRTUAL
  0x50, // KB_P_VIRTUAL
  0x51, // KB_Q_VIRTUAL
  0x52, // KB_R_VIRTUAL
  0x53, // KB_S_VIRTUAL
  0x54, // KB_T_VIRTUAL
  0x55, // KB_U_VIRTUAL
  0x56, // KB_V_VIRTUAL
  0x57, // KB_W_VIRTUAL
  0x58, // KB_X_VIRTUAL
  0x59, // KB_Y_VIRTUAL
  0x5A, // KB_Z_VIRTUAL
  0x00, // KB_F1_VIRTUAL
  0x00, // KB_F2_VIRTUAL
  0x00, // KB_F3_VIRTUAL
  0x00, // KB_F4_VIRTUAL
  0x00, // KB_F5_VIRTUAL
  0x00, // KB_F6_VIRTUAL
  0x00, // KB_F7_VIRTUAL
  0x00, // KB_F8_VIRTUAL
  0x00, // KB_F9_VIRTUAL
  0x00, // KB_F10_VIRTUAL
  0x00, // KB_F11_VIRTUAL
  0x00, // KB_F12_VIRTUAL
  VK_OEM_102, // KB_NONUSBACKSLASH_VIRTUAL
  VK_OEM_COMMA, // KB_COMMA_VIRTUAL
  VK_OEM_PERIOD, // KB_PERIOD_VIRTUAL
  VK_OEM_2, // KB_SLASH_VIRTUAL
  VK_OEM_5, // KB_BACKSLASH_VIRTUAL
  VK_OEM_7, // KB_APOSTROPHE_VIRTUAL
  VK_OEM_1, // KB_SEMICOLON_VIRTUAL
  VK_OEM_6, // KB_RBRACKET_VIRTUAL
  VK_OEM_4, // KB_LBRACKET_VIRTUAL
  0x00, // KB_BACKSPACE_VIRTUAL
  VK_OEM_PLUS, // KB_EQUALS_VIRTUAL
  VK_OEM_MINUS, // KB_MINUS_VIRTUAL
  VK_ESCAPE, // KB_ESCAPE_VIRTUAL
  VK_OEM_3, // KB_GRAVE_VIRTUAL
  0x00, // KB_TAB_VIRTUAL
  0x00, // KB_CAPSLOCK_VIRTUAL
  0x00, // KB_LSHIFT_VIRTUAL
  0x00, // KB_LCTRL_VIRTUAL
  0x00, // KB_LWIN_VIRTUAL
  0x00, // KB_LALT_VIRTUAL
  VK_SPACE, // KB_SPACE_VIRTUAL

  /*
    0x00, // KB_RALT_VIRTUAL
    0x00, // KB_RWIN_VIRTUAL
    0x00, // KB_MENU_VIRTUAL
    0x00, // KB_RCTRL_VIRTUAL
    0x00, // KB_RSHIFT_VIRTUAL
    0x00, // KB_ENTER_VIRTUAL
    0x00, // KB_PRINTSCREEN_VIRTUAL
    0x00, // KB_SCROLLLOCK_VIRTUAL
    0x00, // KB_PAUSE_VIRTUAL
    0x00, // KB_INSERT_VIRTUAL
    0x00, // KB_HOME_VIRTUAL
    0x00, // KB_PAGEUP_VIRTUAL
    0x00, // KB_DELETE_VIRTUAL
    0x00, // KB_END_VIRTUAL
    0x00, // KB_PAGEDOWN_VIRTUAL
    0x00, // KB_UP_VIRTUAL
    0x00, // KB_LEFT_VIRTUAL
    0x00, // KB_DOWN_VIRTUAL
    0x00, // KB_RIGHT_VIRTUAL
    0x00, // KB_NUMLOCK_VIRTUAL
    0x00, // KB_DIVIDE_VIRTUAL
    0x00, // KB_MULTIPLY_VIRTUAL
    0x00, // KB_NUMPAD_MINUS_VIRTUAL
    0x00, // KB_NUMPAD_PLUS_VIRTUAL
    0x00, // KB_NUMPAD_ENTER_VIRTUAL
    0x00, // KB_NUMPAD_DELETE_VIRTUAL
    0x00, // KB_NUMPAD_1_VIRTUAL
    0x00, // KB_NUMPAD_2_VIRTUAL
    0x00, // KB_NUMPAD_3_VIRTUAL
    0x00, // KB_NUMPAD_4_VIRTUAL
    0x00, // KB_NUMPAD_5_VIRTUAL
    0x00, // KB_NUMPAD_6_VIRTUAL
    0x00, // KB_NUMPAD_7_VIRTUAL
    0x00, // KB_NUMPAD_8_VIRTUAL
    0x00, // KB_NUMPAD_9_VIRTUAL
    0x00, // KB_NUMPAD_0_VIRTUAL
    */
};

// Maps virtual key to scancode key
// Will be updated in UpdateVirtualToScancodeMap method
static eInputElements davaVirtualToDavaScancode[] =
{
  eInputElements::KB_1, // KB_1_VIRTUAL
  eInputElements::KB_2, // KB_2_VIRTUAL
  eInputElements::KB_3, // KB_3_VIRTUAL
  eInputElements::KB_4, // KB_4_VIRTUAL
  eInputElements::KB_5, // KB_5_VIRTUAL
  eInputElements::KB_6, // KB_6_VIRTUAL
  eInputElements::KB_7, // KB_7_VIRTUAL
  eInputElements::KB_8, // KB_8_VIRTUAL
  eInputElements::KB_9, // KB_9_VIRTUAL
  eInputElements::KB_0, // KB_0_VIRTUAL
  eInputElements::KB_A, // KB_A_VIRTUAL
  eInputElements::KB_B, // KB_B_VIRTUAL
  eInputElements::KB_C, // KB_C_VIRTUAL
  eInputElements::KB_D, // KB_D_VIRTUAL
  eInputElements::KB_E, // KB_E_VIRTUAL
  eInputElements::KB_F, // KB_F_VIRTUAL
  eInputElements::KB_G, // KB_G_VIRTUAL
  eInputElements::KB_H, // KB_H_VIRTUAL
  eInputElements::KB_I, // KB_I_VIRTUAL
  eInputElements::KB_J, // KB_J_VIRTUAL
  eInputElements::KB_K, // KB_K_VIRTUAL
  eInputElements::KB_L, // KB_L_VIRTUAL
  eInputElements::KB_M, // KB_M_VIRTUAL
  eInputElements::KB_N, // KB_N_VIRTUAL
  eInputElements::KB_O, // KB_O_VIRTUAL
  eInputElements::KB_P, // KB_P_VIRTUAL
  eInputElements::KB_Q, // KB_Q_VIRTUAL
  eInputElements::KB_R, // KB_R_VIRTUAL
  eInputElements::KB_S, // KB_S_VIRTUAL
  eInputElements::KB_T, // KB_T_VIRTUAL
  eInputElements::KB_U, // KB_U_VIRTUAL
  eInputElements::KB_V, // KB_V_VIRTUAL
  eInputElements::KB_W, // KB_W_VIRTUAL
  eInputElements::KB_X, // KB_X_VIRTUAL
  eInputElements::KB_Y, // KB_Y_VIRTUAL
  eInputElements::KB_Z, // KB_Z_VIRTUAL
  eInputElements::KB_F1, // KB_F1_VIRTUAL
  eInputElements::KB_F2, // KB_F2_VIRTUAL
  eInputElements::KB_F3, // KB_F3_VIRTUAL
  eInputElements::KB_F4, // KB_F4_VIRTUAL
  eInputElements::KB_F5, // KB_F5_VIRTUAL
  eInputElements::KB_F6, // KB_F6_VIRTUAL
  eInputElements::KB_F7, // KB_F7_VIRTUAL
  eInputElements::KB_F8, // KB_F8_VIRTUAL
  eInputElements::KB_F9, // KB_F9_VIRTUAL
  eInputElements::KB_F10, // KB_F10_VIRTUAL
  eInputElements::KB_F11, // KB_F11_VIRTUAL
  eInputElements::KB_F12, // KB_F12_VIRTUAL
  eInputElements::KB_NONUSBACKSLASH, // KB_NONUSBACKSLASH_VIRTUAL
  eInputElements::KB_COMMA, // KB_COMMA_VIRTUAL
  eInputElements::KB_PERIOD, // KB_PERIOD_VIRTUAL
  eInputElements::KB_SLASH, // KB_SLASH_VIRTUAL
  eInputElements::KB_BACKSLASH, // KB_BACKSLASH_VIRTUAL
  eInputElements::KB_APOSTROPHE, // KB_APOSTROPHE_VIRTUAL
  eInputElements::KB_SEMICOLON, // KB_SEMICOLON_VIRTUAL
  eInputElements::KB_RBRACKET, // KB_RBRACKET_VIRTUAL
  eInputElements::KB_LBRACKET, // KB_LBRACKET_VIRTUAL
  eInputElements::KB_BACKSPACE, // KB_BACKSPACE_VIRTUAL
  eInputElements::KB_EQUALS, // KB_EQUALS_VIRTUAL
  eInputElements::KB_MINUS, // KB_MINUS_VIRTUAL
  eInputElements::KB_ESCAPE, // KB_ESCAPE_VIRTUAL
  eInputElements::KB_GRAVE, // KB_GRAVE_VIRTUAL
  eInputElements::KB_TAB, // KB_TAB_VIRTUAL
  eInputElements::KB_CAPSLOCK, // KB_CAPSLOCK_VIRTUAL
  eInputElements::KB_LSHIFT, // KB_LSHIFT_VIRTUAL
  eInputElements::KB_LCTRL, // KB_LCTRL_VIRTUAL
  eInputElements::KB_LWIN, // KB_LWIN_VIRTUAL
  eInputElements::KB_LALT, // KB_LALT_VIRTUAL
  eInputElements::KB_SPACE, // KB_SPACE_VIRTUAL
  eInputElements::KB_RALT, // KB_RALT_VIRTUAL
  eInputElements::KB_RWIN, // KB_RWIN_VIRTUAL
  eInputElements::KB_MENU, // KB_MENU_VIRTUAL
  eInputElements::KB_RCTRL, // KB_RCTRL_VIRTUAL
  eInputElements::KB_RSHIFT, // KB_RSHIFT_VIRTUAL
  eInputElements::KB_ENTER, // KB_ENTER_VIRTUAL
  eInputElements::KB_PRINTSCREEN, // KB_PRINTSCREEN_VIRTUAL
  eInputElements::KB_SCROLLLOCK, // KB_SCROLLLOCK_VIRTUAL
  eInputElements::KB_PAUSE, // KB_PAUSE_VIRTUAL
  eInputElements::KB_INSERT, // KB_INSERT_VIRTUAL
  eInputElements::KB_HOME, // KB_HOME_VIRTUAL
  eInputElements::KB_PAGEUP, // KB_PAGEUP_VIRTUAL
  eInputElements::KB_DELETE, // KB_DELETE_VIRTUAL
  eInputElements::KB_END, // KB_END_VIRTUAL
  eInputElements::KB_PAGEDOWN, // KB_PAGEDOWN_VIRTUAL
  eInputElements::KB_UP, // KB_UP_VIRTUAL
  eInputElements::KB_LEFT, // KB_LEFT_VIRTUAL
  eInputElements::KB_DOWN, // KB_DOWN_VIRTUAL
  eInputElements::KB_RIGHT, // KB_RIGHT_VIRTUAL
  eInputElements::KB_NUMLOCK, // KB_NUMLOCK_VIRTUAL
  eInputElements::KB_DIVIDE, // KB_DIVIDE_VIRTUAL
  eInputElements::KB_MULTIPLY, // KB_MULTIPLY_VIRTUAL
  eInputElements::KB_NUMPAD_MINUS, // KB_NUMPAD_MINUS_VIRTUAL
  eInputElements::KB_NUMPAD_PLUS, // KB_NUMPAD_PLUS_VIRTUAL
  eInputElements::KB_NUMPAD_ENTER, // KB_NUMPAD_ENTER_VIRTUAL
  eInputElements::KB_NUMPAD_DELETE, // KB_NUMPAD_DELETE_VIRTUAL
  eInputElements::KB_NUMPAD_1, // KB_NUMPAD_1_VIRTUAL
  eInputElements::KB_NUMPAD_2, // KB_NUMPAD_2_VIRTUAL
  eInputElements::KB_NUMPAD_3, // KB_NUMPAD_3_VIRTUAL
  eInputElements::KB_NUMPAD_4, // KB_NUMPAD_4_VIRTUAL
  eInputElements::KB_NUMPAD_5, // KB_NUMPAD_5_VIRTUAL
  eInputElements::KB_NUMPAD_6, // KB_NUMPAD_6_VIRTUAL
  eInputElements::KB_NUMPAD_7, // KB_NUMPAD_7_VIRTUAL
  eInputElements::KB_NUMPAD_8, // KB_NUMPAD_8_VIRTUAL
  eInputElements::KB_NUMPAD_9, // KB_NUMPAD_9_VIRTUAL
  eInputElements::KB_NUMPAD_0, // KB_NUMPAD_0_VIRTUAL
};

KeyboardDeviceImpl::KeyboardDeviceImpl()
{
    UpdateVirtualToScancodeMap();

    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &KeyboardDeviceImpl::HandleEvent));
}

KeyboardDeviceImpl::~KeyboardDeviceImpl()
{
    Private::EngineBackend::Instance()->UninstallEventFilter(this);
}

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

    for (size_t i = 0; i < COUNT_OF(davaVirtualToDavaScancode); ++i)
    {
        if (davaVirtualToDavaScancode[i] == scancodeElement)
        {
            return static_cast<eInputElements>(eInputElements::KB_FIRST_VIRTUAL + i);
        }
    }

    return eInputElements::NONE;
}

eInputElements KeyboardDeviceImpl::ConvertDavaVirtualToDavaScancode(eInputElements virtualElement)
{
    DVASSERT(IsKeyboardVirtualInputElement(virtualElement));

    return davaVirtualToDavaScancode[virtualElement - KB_FIRST_VIRTUAL];
}

bool KeyboardDeviceImpl::HandleEvent(const Private::MainDispatcherEvent& e)
{
    if (e.type == MainDispatcherEvent::INPUT_LANGUAGE_CHANGED)
    {
        UpdateVirtualToScancodeMap();
    }

    return false;
}

void KeyboardDeviceImpl::UpdateVirtualToScancodeMap()
{
    for (size_t i = 0; i < COUNT_OF(davaVirtualMappableToNativeVirtual); ++i)
    {
        const uint32 nativeVirtual = davaVirtualMappableToNativeVirtual[i];
        if (nativeVirtual != 0)
        {
            const uint32 nativeScancode = MapVirtualKey(nativeVirtual, MAPVK_VK_TO_VSC);
            if (nativeScancode != 0)
            {
                eInputElements davaScancode = ConvertNativeScancodeToDavaScancode(nativeScancode);
                davaVirtualToDavaScancode[i] = davaScancode;
            }
        }
    }
}

String KeyboardDeviceImpl::GetElementStringRepresentation(eInputElements elementId)
{
    // Win32 supports virtual keys, so map scancode to virtual and return virtual key name

    if (IsKeyboardScancodeInputElement(elementId))
    {
        elementId = ConvertDavaScancodeToDavaVirtual(elementId);
    }

    InputElementInfo virtualElementInfo = GetInputElementInfo(elementId);
    return virtualElementInfo.name;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
