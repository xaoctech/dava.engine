#include "Input/Private/Win32/KeyboardDeviceImplWin32.h"

#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include <Windows.h>

namespace DAVA
{
namespace Private
{
static const eInputElements nativeScancodeToDavaScancode[] =
{
  eInputElements::NONE, // 0x00
  eInputElements::KB_ESCAPE_SCANCODE, // 0x01
  eInputElements::KB_1_SCANCODE, // 0x02
  eInputElements::KB_2_SCANCODE, // 0x03
  eInputElements::KB_3_SCANCODE, // 0x04
  eInputElements::KB_4_SCANCODE, // 0x05
  eInputElements::KB_5_SCANCODE, // 0x06
  eInputElements::KB_6_SCANCODE, // 0x07
  eInputElements::KB_7_SCANCODE, // 0x08
  eInputElements::KB_8_SCANCODE, // 0x09
  eInputElements::KB_9_SCANCODE, // 0x0A
  eInputElements::KB_0_SCANCODE, // 0x0B
  eInputElements::KB_MINUS_SCANCODE, // 0x0C
  eInputElements::KB_EQUALS_SCANCODE, // 0x0D
  eInputElements::KB_BACKSPACE_SCANCODE, // 0x0E
  eInputElements::KB_TAB_SCANCODE, // 0x0F
  eInputElements::KB_Q_SCANCODE, // 0x10
  eInputElements::KB_W_SCANCODE, // 0x11
  eInputElements::KB_E_SCANCODE, // 0x12
  eInputElements::KB_R_SCANCODE, // 0x13
  eInputElements::KB_T_SCANCODE, // 0x14
  eInputElements::KB_Y_SCANCODE, // 0x15
  eInputElements::KB_U_SCANCODE, // 0x16
  eInputElements::KB_I_SCANCODE, // 0x17
  eInputElements::KB_O_SCANCODE, // 0x18
  eInputElements::KB_P_SCANCODE, // 0x19
  eInputElements::KB_LBRACKET_SCANCODE, // 0x1A
  eInputElements::KB_RBRACKET_SCANCODE, // 0x1B
  eInputElements::KB_ENTER_SCANCODE, // 0x1C
  eInputElements::KB_LCTRL_SCANCODE, // 0x1D
  eInputElements::KB_A_SCANCODE, // 0x1E
  eInputElements::KB_S_SCANCODE, // 0x1F
  eInputElements::KB_D_SCANCODE, // 0x20
  eInputElements::KB_F_SCANCODE, // 0x21
  eInputElements::KB_G_SCANCODE, // 0x22
  eInputElements::KB_H_SCANCODE, // 0x23
  eInputElements::KB_J_SCANCODE, // 0x24
  eInputElements::KB_K_SCANCODE, // 0x25
  eInputElements::KB_L_SCANCODE, // 0x26
  eInputElements::KB_SEMICOLON_SCANCODE, // 0x27
  eInputElements::KB_APOSTROPHE_SCANCODE, // 0x28
  eInputElements::KB_GRAVE_SCANCODE, // 0x29
  eInputElements::KB_LSHIFT_SCANCODE, // 0x2A
  eInputElements::KB_BACKSLASH_SCANCODE, // 0x2B
  eInputElements::KB_Z_SCANCODE, // 0x2C
  eInputElements::KB_X_SCANCODE, // 0x2D
  eInputElements::KB_C_SCANCODE, // 0x2E
  eInputElements::KB_V_SCANCODE, // 0x2F
  eInputElements::KB_B_SCANCODE, // 0x30
  eInputElements::KB_N_SCANCODE, // 0x31
  eInputElements::KB_M_SCANCODE, // 0x32
  eInputElements::KB_COMMA_SCANCODE, // 0x33
  eInputElements::KB_PERIOD_SCANCODE, // 0x34
  eInputElements::KB_SLASH_SCANCODE, // 0x35
  eInputElements::KB_RSHIFT_SCANCODE, // 0x36
  eInputElements::KB_MULTIPLY_SCANCODE, // 0x37
  eInputElements::KB_LALT_SCANCODE, // 0x38
  eInputElements::KB_SPACE_SCANCODE, // 0x39
  eInputElements::KB_CAPSLOCK_SCANCODE, // 0x3A
  eInputElements::KB_F1_SCANCODE, // 0x3B
  eInputElements::KB_F2_SCANCODE, // 0x3C
  eInputElements::KB_F3_SCANCODE, // 0x3D
  eInputElements::KB_F4_SCANCODE, // 0x3E
  eInputElements::KB_F5_SCANCODE, // 0x3F
  eInputElements::KB_F6_SCANCODE, // 0x40
  eInputElements::KB_F7_SCANCODE, // 0x41
  eInputElements::KB_F8_SCANCODE, // 0x42
  eInputElements::KB_F9_SCANCODE, // 0x43
  eInputElements::KB_F10_SCANCODE, // 0x44
  eInputElements::KB_PAUSE_SCANCODE, // 0x45
  eInputElements::KB_SCROLLLOCK_SCANCODE, // 0x46
  eInputElements::KB_NUMPAD_7_SCANCODE, // 0x47
  eInputElements::KB_NUMPAD_8_SCANCODE, // 0x48
  eInputElements::KB_NUMPAD_9_SCANCODE, // 0x49
  eInputElements::KB_NUMPAD_MINUS_SCANCODE, // 0x4A
  eInputElements::KB_NUMPAD_4_SCANCODE, // 0x4B
  eInputElements::KB_NUMPAD_5_SCANCODE, // 0x4C
  eInputElements::KB_NUMPAD_6_SCANCODE, // 0x4D
  eInputElements::KB_NUMPAD_PLUS_SCANCODE, // 0x4E
  eInputElements::KB_NUMPAD_1_SCANCODE, // 0x4F
  eInputElements::KB_NUMPAD_2_SCANCODE, // 0x50
  eInputElements::KB_NUMPAD_3_SCANCODE, // 0x51
  eInputElements::KB_NUMPAD_0_SCANCODE, // 0x52
  eInputElements::KB_NUMPAD_DELETE_SCANCODE, // 0x53
  eInputElements::NONE, // 0x54
  eInputElements::NONE, // 0x55
  eInputElements::KB_NONUSBACKSLASH_SCANCODE, // 0x56
  eInputElements::KB_F11_SCANCODE, // 0x57
  eInputElements::KB_F12_SCANCODE, // 0x58
};

static const eInputElements nativeScancodeExtToDavaScancode[] =
{
  eInputElements::NONE, // 0x00
  eInputElements::NONE, // 0x01
  eInputElements::NONE, // 0x02
  eInputElements::NONE, // 0x03
  eInputElements::NONE, // 0x04
  eInputElements::NONE, // 0x05
  eInputElements::NONE, // 0x06
  eInputElements::NONE, // 0x07
  eInputElements::NONE, // 0x08
  eInputElements::NONE, // 0x09
  eInputElements::NONE, // 0x0A
  eInputElements::NONE, // 0x0B
  eInputElements::NONE, // 0x0C
  eInputElements::NONE, // 0x0D
  eInputElements::NONE, // 0x0E
  eInputElements::NONE, // 0x0F
  eInputElements::NONE, // 0x10
  eInputElements::NONE, // 0x11
  eInputElements::NONE, // 0x12
  eInputElements::NONE, // 0x13
  eInputElements::NONE, // 0x14
  eInputElements::NONE, // 0x15
  eInputElements::NONE, // 0x16
  eInputElements::NONE, // 0x17
  eInputElements::NONE, // 0x18
  eInputElements::NONE, // 0x19
  eInputElements::NONE, // 0x1A
  eInputElements::NONE, // 0x1B
  eInputElements::KB_NUMPAD_ENTER_SCANCODE, // 0x1C
  eInputElements::KB_RCTRL_SCANCODE, // 0x1D
  eInputElements::NONE, // 0x1E
  eInputElements::NONE, // 0x1F
  eInputElements::NONE, // 0x20
  eInputElements::NONE, // 0x21
  eInputElements::NONE, // 0x22
  eInputElements::NONE, // 0x23
  eInputElements::NONE, // 0x24
  eInputElements::NONE, // 0x25
  eInputElements::NONE, // 0x26
  eInputElements::NONE, // 0x27
  eInputElements::NONE, // 0x28
  eInputElements::NONE, // 0x29
  eInputElements::NONE, // 0x2A
  eInputElements::NONE, // 0x2B
  eInputElements::NONE, // 0x2C
  eInputElements::NONE, // 0x2D
  eInputElements::NONE, // 0x2E
  eInputElements::NONE, // 0x2F
  eInputElements::NONE, // 0x30
  eInputElements::NONE, // 0x31
  eInputElements::NONE, // 0x32
  eInputElements::NONE, // 0x33
  eInputElements::NONE, // 0x34
  eInputElements::KB_DIVIDE_SCANCODE, // 0x35
  eInputElements::NONE, // 0x36
  eInputElements::KB_PRINTSCREEN_SCANCODE, // 0x37
  eInputElements::KB_RALT_SCANCODE, // 0x38
  eInputElements::NONE, // 0x39
  eInputElements::NONE, // 0x3A
  eInputElements::NONE, // 0x3B
  eInputElements::NONE, // 0x3C
  eInputElements::NONE, // 0x3D
  eInputElements::NONE, // 0x3E
  eInputElements::NONE, // 0x3F
  eInputElements::NONE, // 0x40
  eInputElements::NONE, // 0x41
  eInputElements::NONE, // 0x42
  eInputElements::NONE, // 0x43
  eInputElements::NONE, // 0x44
  eInputElements::KB_NUMLOCK_SCANCODE, // 0x45
  eInputElements::NONE, // 0x46
  eInputElements::KB_HOME_SCANCODE, // 0x47
  eInputElements::KB_UP_SCANCODE, // 0x48
  eInputElements::KB_PAGEUP_SCANCODE, // 0x49
  eInputElements::NONE, // 0x4A
  eInputElements::KB_LEFT_SCANCODE, // 0x4B
  eInputElements::NONE, // 0x4C
  eInputElements::KB_RIGHT_SCANCODE, // 0x4D
  eInputElements::NONE, // 0x4E
  eInputElements::KB_END_SCANCODE, // 0x4F
  eInputElements::KB_DOWN_SCANCODE, // 0x50
  eInputElements::KB_PAGEDOWN_SCANCODE, // 0x51
  eInputElements::KB_INSERT_SCANCODE, // 0x52
  eInputElements::KB_DELETE_SCANCODE, // 0x53
  eInputElements::NONE, // 0x54
  eInputElements::NONE, // 0x55
  eInputElements::NONE, // 0x56
  eInputElements::NONE, // 0x57
  eInputElements::NONE, // 0x58
  eInputElements::NONE, // 0x59
  eInputElements::NONE, // 0x5A
  eInputElements::KB_LWIN_SCANCODE, // 0x5B
  eInputElements::KB_RWIN_SCANCODE, // 0x5C
  eInputElements::KB_MENU_SCANCODE, // 0x5D
};

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

static eInputElements davaVirtualToDavaScancode[] =
{
  eInputElements::KB_1_SCANCODE, // KB_1_VIRTUAL
  eInputElements::KB_2_SCANCODE, // KB_2_VIRTUAL
  eInputElements::KB_3_SCANCODE, // KB_3_VIRTUAL
  eInputElements::KB_4_SCANCODE, // KB_4_VIRTUAL
  eInputElements::KB_5_SCANCODE, // KB_5_VIRTUAL
  eInputElements::KB_6_SCANCODE, // KB_6_VIRTUAL
  eInputElements::KB_7_SCANCODE, // KB_7_VIRTUAL
  eInputElements::KB_8_SCANCODE, // KB_8_VIRTUAL
  eInputElements::KB_9_SCANCODE, // KB_9_VIRTUAL
  eInputElements::KB_0_SCANCODE, // KB_0_VIRTUAL
  eInputElements::KB_A_SCANCODE, // KB_A_VIRTUAL
  eInputElements::KB_B_SCANCODE, // KB_B_VIRTUAL
  eInputElements::KB_C_SCANCODE, // KB_C_VIRTUAL
  eInputElements::KB_D_SCANCODE, // KB_D_VIRTUAL
  eInputElements::KB_E_SCANCODE, // KB_E_VIRTUAL
  eInputElements::KB_F_SCANCODE, // KB_F_VIRTUAL
  eInputElements::KB_G_SCANCODE, // KB_G_VIRTUAL
  eInputElements::KB_H_SCANCODE, // KB_H_VIRTUAL
  eInputElements::KB_I_SCANCODE, // KB_I_VIRTUAL
  eInputElements::KB_J_SCANCODE, // KB_J_VIRTUAL
  eInputElements::KB_K_SCANCODE, // KB_K_VIRTUAL
  eInputElements::KB_L_SCANCODE, // KB_L_VIRTUAL
  eInputElements::KB_M_SCANCODE, // KB_M_VIRTUAL
  eInputElements::KB_N_SCANCODE, // KB_N_VIRTUAL
  eInputElements::KB_O_SCANCODE, // KB_O_VIRTUAL
  eInputElements::KB_P_SCANCODE, // KB_P_VIRTUAL
  eInputElements::KB_Q_SCANCODE, // KB_Q_VIRTUAL
  eInputElements::KB_R_SCANCODE, // KB_R_VIRTUAL
  eInputElements::KB_S_SCANCODE, // KB_S_VIRTUAL
  eInputElements::KB_T_SCANCODE, // KB_T_VIRTUAL
  eInputElements::KB_U_SCANCODE, // KB_U_VIRTUAL
  eInputElements::KB_V_SCANCODE, // KB_V_VIRTUAL
  eInputElements::KB_W_SCANCODE, // KB_W_VIRTUAL
  eInputElements::KB_X_SCANCODE, // KB_X_VIRTUAL
  eInputElements::KB_Y_SCANCODE, // KB_Y_VIRTUAL
  eInputElements::KB_Z_SCANCODE, // KB_Z_VIRTUAL
  eInputElements::KB_F1_SCANCODE, // KB_F1_VIRTUAL
  eInputElements::KB_F2_SCANCODE, // KB_F2_VIRTUAL
  eInputElements::KB_F3_SCANCODE, // KB_F3_VIRTUAL
  eInputElements::KB_F4_SCANCODE, // KB_F4_VIRTUAL
  eInputElements::KB_F5_SCANCODE, // KB_F5_VIRTUAL
  eInputElements::KB_F6_SCANCODE, // KB_F6_VIRTUAL
  eInputElements::KB_F7_SCANCODE, // KB_F7_VIRTUAL
  eInputElements::KB_F8_SCANCODE, // KB_F8_VIRTUAL
  eInputElements::KB_F9_SCANCODE, // KB_F9_VIRTUAL
  eInputElements::KB_F10_SCANCODE, // KB_F10_VIRTUAL
  eInputElements::KB_F11_SCANCODE, // KB_F11_VIRTUAL
  eInputElements::KB_F12_SCANCODE, // KB_F12_VIRTUAL
  eInputElements::KB_NONUSBACKSLASH_SCANCODE, // KB_NONUSBACKSLASH_VIRTUAL
  eInputElements::KB_COMMA_SCANCODE, // KB_COMMA_VIRTUAL
  eInputElements::KB_PERIOD_SCANCODE, // KB_PERIOD_VIRTUAL
  eInputElements::KB_SLASH_SCANCODE, // KB_SLASH_VIRTUAL
  eInputElements::KB_BACKSLASH_SCANCODE, // KB_BACKSLASH_VIRTUAL
  eInputElements::KB_APOSTROPHE_SCANCODE, // KB_APOSTROPHE_VIRTUAL
  eInputElements::KB_SEMICOLON_SCANCODE, // KB_SEMICOLON_VIRTUAL
  eInputElements::KB_RBRACKET_SCANCODE, // KB_RBRACKET_VIRTUAL
  eInputElements::KB_LBRACKET_SCANCODE, // KB_LBRACKET_VIRTUAL
  eInputElements::KB_BACKSPACE_SCANCODE, // KB_BACKSPACE_VIRTUAL
  eInputElements::KB_EQUALS_SCANCODE, // KB_EQUALS_VIRTUAL
  eInputElements::KB_MINUS_SCANCODE, // KB_MINUS_VIRTUAL
  eInputElements::KB_ESCAPE_SCANCODE, // KB_ESCAPE_VIRTUAL
  eInputElements::KB_GRAVE_SCANCODE, // KB_GRAVE_VIRTUAL
  eInputElements::KB_TAB_SCANCODE, // KB_TAB_VIRTUAL
  eInputElements::KB_CAPSLOCK_SCANCODE, // KB_CAPSLOCK_VIRTUAL
  eInputElements::KB_LSHIFT_SCANCODE, // KB_LSHIFT_VIRTUAL
  eInputElements::KB_LCTRL_SCANCODE, // KB_LCTRL_VIRTUAL
  eInputElements::KB_LWIN_SCANCODE, // KB_LWIN_VIRTUAL
  eInputElements::KB_LALT_SCANCODE, // KB_LALT_VIRTUAL
  eInputElements::KB_SPACE_SCANCODE, // KB_SPACE_VIRTUAL
  eInputElements::KB_RALT_SCANCODE, // KB_RALT_VIRTUAL
  eInputElements::KB_RWIN_SCANCODE, // KB_RWIN_VIRTUAL
  eInputElements::KB_MENU_SCANCODE, // KB_MENU_VIRTUAL
  eInputElements::KB_RCTRL_SCANCODE, // KB_RCTRL_VIRTUAL
  eInputElements::KB_RSHIFT_SCANCODE, // KB_RSHIFT_VIRTUAL
  eInputElements::KB_ENTER_SCANCODE, // KB_ENTER_VIRTUAL
  eInputElements::KB_PRINTSCREEN_SCANCODE, // KB_PRINTSCREEN_VIRTUAL
  eInputElements::KB_SCROLLLOCK_SCANCODE, // KB_SCROLLLOCK_VIRTUAL
  eInputElements::KB_PAUSE_SCANCODE, // KB_PAUSE_VIRTUAL
  eInputElements::KB_INSERT_SCANCODE, // KB_INSERT_VIRTUAL
  eInputElements::KB_HOME_SCANCODE, // KB_HOME_VIRTUAL
  eInputElements::KB_PAGEUP_SCANCODE, // KB_PAGEUP_VIRTUAL
  eInputElements::KB_DELETE_SCANCODE, // KB_DELETE_VIRTUAL
  eInputElements::KB_END_SCANCODE, // KB_END_VIRTUAL
  eInputElements::KB_PAGEDOWN_SCANCODE, // KB_PAGEDOWN_VIRTUAL
  eInputElements::KB_UP_SCANCODE, // KB_UP_VIRTUAL
  eInputElements::KB_LEFT_SCANCODE, // KB_LEFT_VIRTUAL
  eInputElements::KB_DOWN_SCANCODE, // KB_DOWN_VIRTUAL
  eInputElements::KB_RIGHT_SCANCODE, // KB_RIGHT_VIRTUAL
  eInputElements::KB_NUMLOCK_SCANCODE, // KB_NUMLOCK_VIRTUAL
  eInputElements::KB_DIVIDE_SCANCODE, // KB_DIVIDE_VIRTUAL
  eInputElements::KB_MULTIPLY_SCANCODE, // KB_MULTIPLY_VIRTUAL
  eInputElements::KB_NUMPAD_MINUS_SCANCODE, // KB_NUMPAD_MINUS_VIRTUAL
  eInputElements::KB_NUMPAD_PLUS_SCANCODE, // KB_NUMPAD_PLUS_VIRTUAL
  eInputElements::KB_NUMPAD_ENTER_SCANCODE, // KB_NUMPAD_ENTER_VIRTUAL
  eInputElements::KB_NUMPAD_DELETE_SCANCODE, // KB_NUMPAD_DELETE_VIRTUAL
  eInputElements::KB_NUMPAD_1_SCANCODE, // KB_NUMPAD_1_VIRTUAL
  eInputElements::KB_NUMPAD_2_SCANCODE, // KB_NUMPAD_2_VIRTUAL
  eInputElements::KB_NUMPAD_3_SCANCODE, // KB_NUMPAD_3_VIRTUAL
  eInputElements::KB_NUMPAD_4_SCANCODE, // KB_NUMPAD_4_VIRTUAL
  eInputElements::KB_NUMPAD_5_SCANCODE, // KB_NUMPAD_5_VIRTUAL
  eInputElements::KB_NUMPAD_6_SCANCODE, // KB_NUMPAD_6_VIRTUAL
  eInputElements::KB_NUMPAD_7_SCANCODE, // KB_NUMPAD_7_VIRTUAL
  eInputElements::KB_NUMPAD_8_SCANCODE, // KB_NUMPAD_8_VIRTUAL
  eInputElements::KB_NUMPAD_9_SCANCODE, // KB_NUMPAD_9_VIRTUAL
  eInputElements::KB_NUMPAD_0_SCANCODE, // KB_NUMPAD_0_VIRTUAL
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
    DVASSERT(scancodeElement >= eInputElements::KB_FIRST_SCANCODE && scancodeElement <= eInputElements::KB_LAST_SCANCODE);

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
    DVASSERT(virtualElement >= eInputElements::KB_FIRST_VIRTUAL && virtualElement <= eInputElements::KB_LAST_VIRTUAL);

    return davaVirtualToDavaScancode[virtualElement - KB_FIRST_VIRTUAL];
}

bool KeyboardDeviceImpl::HandleEvent(const Private::MainDispatcherEvent& e)
{
    if (e.type == MainDispatcherEvent::INPUT_LANGUAGE_CHANGED)
    {
        UpdateVirtualToScancodeMap();
        return true;
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
}
}