#include "Input/Private/Mac/KeyboardDeviceImplMac.h"

#if defined (__DAVAENGINE_MACOS__)

#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"
#include "Engine/Engine.h"

namespace DAVA
{
namespace Private
{
const eInputElements nativeScancodeToDavaScancode[] =
{
  eInputElements::KB_A_SCANCODE, // 0x00
  eInputElements::KB_S_SCANCODE, // 0x01
  eInputElements::KB_D_SCANCODE, // 0x02
  eInputElements::KB_F_SCANCODE, // 0x03
  eInputElements::KB_H_SCANCODE, // 0x04
  eInputElements::KB_G_SCANCODE, // 0x05
  eInputElements::KB_Z_SCANCODE, // 0x06
  eInputElements::KB_X_SCANCODE, // 0x07
  eInputElements::KB_C_SCANCODE, // 0x08
  eInputElements::KB_V_SCANCODE, // 0x09
  eInputElements::KB_NONUSBACKSLASH_SCANCODE, // 0x0A
  eInputElements::KB_B_SCANCODE, // 0x0B
  eInputElements::KB_Q_SCANCODE, // 0x0C
  eInputElements::KB_W_SCANCODE, // 0x0D
  eInputElements::KB_E_SCANCODE, // 0x0E
  eInputElements::KB_R_SCANCODE, // 0x0F
  eInputElements::KB_Y_SCANCODE, // 0x10
  eInputElements::KB_T_SCANCODE, // 0x11
  eInputElements::KB_1_SCANCODE, // 0x12
  eInputElements::KB_2_SCANCODE, // 0x13
  eInputElements::KB_3_SCANCODE, // 0x14
  eInputElements::KB_4_SCANCODE, // 0x15
  eInputElements::KB_6_SCANCODE, // 0x16
  eInputElements::KB_5_SCANCODE, // 0x17
  eInputElements::KB_EQUALS_SCANCODE, // 0x18
  eInputElements::KB_9_SCANCODE, // 0x19
  eInputElements::KB_7_SCANCODE, // 0x1A
  eInputElements::KB_MINUS_SCANCODE, // 0x1B
  eInputElements::KB_8_SCANCODE, // 0x1C
  eInputElements::KB_0_SCANCODE, // 0x1D
  eInputElements::KB_RBRACKET_SCANCODE, // 0x1E
  eInputElements::KB_O_SCANCODE, // 0x1F
  eInputElements::KB_U_SCANCODE, // 0x20
  eInputElements::KB_LBRACKET_SCANCODE, // 0x21
  eInputElements::KB_I_SCANCODE, // 0x22
  eInputElements::KB_P_SCANCODE, // 0x23
  eInputElements::KB_ENTER_SCANCODE, // 0x24
  eInputElements::KB_L_SCANCODE, // 0x25
  eInputElements::KB_J_SCANCODE, // 0x26
  eInputElements::KB_APOSTROPHE_SCANCODE, // 0x27
  eInputElements::KB_K_SCANCODE, // 0x28
  eInputElements::KB_SEMICOLON_SCANCODE, // 0x29
  eInputElements::KB_BACKSLASH_SCANCODE, // 0x2A
  eInputElements::KB_COMMA_SCANCODE, // 0x2B
  eInputElements::KB_SLASH_SCANCODE, // 0x2C
  eInputElements::KB_N_SCANCODE, // 0x2D
  eInputElements::KB_M_SCANCODE, // 0x2E
  eInputElements::KB_PERIOD_SCANCODE, // 0x2F
  eInputElements::KB_TAB_SCANCODE, // 0x30
  eInputElements::KB_SPACE_SCANCODE, // 0x31
  eInputElements::KB_GRAVE_SCANCODE, // 0x32
  eInputElements::KB_BACKSPACE_SCANCODE, // 0x33
  eInputElements::NONE, // 0x34
  eInputElements::KB_ESCAPE_SCANCODE, // 0x35
  eInputElements::KB_RCMD_SCANCODE, // 0x36
  eInputElements::KB_LCMD_SCANCODE, // 0x37
  eInputElements::KB_LSHIFT_SCANCODE, // 0x38
  eInputElements::KB_CAPSLOCK_SCANCODE, // 0x39
  eInputElements::KB_LALT_SCANCODE, // 0x3A
  eInputElements::KB_LCTRL_SCANCODE, // 0x3B
  eInputElements::KB_RSHIFT_SCANCODE, // 0x3C
  eInputElements::KB_RALT_SCANCODE, // 0x3D
  eInputElements::KB_RCTRL_SCANCODE, // 0x3E
  eInputElements::NONE, // 0x3F
  eInputElements::NONE, // 0x40
  eInputElements::KB_NUMPAD_DELETE_SCANCODE, // 0x41
  eInputElements::NONE, // 0x42
  eInputElements::KB_MULTIPLY_SCANCODE, // 0x43
  eInputElements::NONE, // 0x44
  eInputElements::KB_NUMPAD_PLUS_SCANCODE, // 0x45
  eInputElements::NONE, // 0x46
  eInputElements::NONE, // 0x47
  eInputElements::NONE, // 0x48
  eInputElements::NONE, // 0x49
  eInputElements::NONE, // 0x4A
  eInputElements::KB_DIVIDE_SCANCODE, // 0x4B
  eInputElements::KB_NUMPAD_ENTER_SCANCODE, // 0x4C
  eInputElements::NONE, // 0x4D
  eInputElements::KB_NUMPAD_MINUS_SCANCODE, // 0x4E
  eInputElements::NONE, // 0x4F
  eInputElements::NONE, // 0x50
  eInputElements::NONE, // 0x51
  eInputElements::KB_NUMPAD_0_SCANCODE, // 0x52
  eInputElements::KB_NUMPAD_1_SCANCODE, // 0x53
  eInputElements::KB_NUMPAD_2_SCANCODE, // 0x54
  eInputElements::KB_NUMPAD_3_SCANCODE, // 0x55
  eInputElements::KB_NUMPAD_4_SCANCODE, // 0x56
  eInputElements::KB_NUMPAD_5_SCANCODE, // 0x57
  eInputElements::KB_NUMPAD_6_SCANCODE, // 0x58
  eInputElements::KB_NUMPAD_7_SCANCODE, // 0x59
  eInputElements::NONE, // 0x5A
  eInputElements::KB_NUMPAD_8_SCANCODE, // 0x5B
  eInputElements::KB_NUMPAD_9_SCANCODE, // 0x5C
  eInputElements::NONE, // 0x5D
  eInputElements::NONE, // 0x5E
  eInputElements::NONE, // 0x5F
  eInputElements::KB_F5_SCANCODE, // 0x60
  eInputElements::KB_F6_SCANCODE, // 0x61
  eInputElements::KB_F7_SCANCODE, // 0x62
  eInputElements::KB_F3_SCANCODE, // 0x63
  eInputElements::KB_F8_SCANCODE, // 0x64
  eInputElements::KB_F9_SCANCODE, // 0x65
  eInputElements::NONE, // 0x66
  eInputElements::KB_F11_SCANCODE, // 0x67
  eInputElements::NONE, // 0x68
  eInputElements::NONE, // 0x69
  eInputElements::NONE, // 0x6A
  eInputElements::NONE, // 0x6B
  eInputElements::NONE, // 0x6C
  eInputElements::KB_F10_SCANCODE, // 0x6D
  eInputElements::NONE, // 0x6E
  eInputElements::KB_F12_SCANCODE, // 0x6F
  eInputElements::NONE, // 0x70
  eInputElements::NONE, // 0x71
  eInputElements::NONE, // 0x72
  eInputElements::KB_HOME_SCANCODE, // 0x73
  eInputElements::KB_PAGEUP_SCANCODE, // 0x74
  eInputElements::NONE, // 0x75
  eInputElements::KB_F4_SCANCODE, // 0x76
  eInputElements::KB_END_SCANCODE, // 0x77
  eInputElements::KB_F2_SCANCODE, // 0x78
  eInputElements::KB_PAGEDOWN_SCANCODE, // 0x79
  eInputElements::KB_F1_SCANCODE, // 0x7A
  eInputElements::KB_LEFT_SCANCODE, // 0x7B
  eInputElements::KB_RIGHT_SCANCODE, // 0x7C
  eInputElements::KB_DOWN_SCANCODE, // 0x7D
  eInputElements::KB_UP_SCANCODE, // 0x7E
};

eInputElements KeyboardDeviceImpl::ConvertNativeScancodeToDavaScancode(uint32 nativeScancode)
{
    return nativeScancodeToDavaScancode[nativeScancode];
}

eInputElements KeyboardDeviceImpl::ConvertDavaScancodeToDavaVirtual(eInputElements scancodeElement)
{
    DVASSERT(scancodeElement >= eInputElements::KB_FIRST_SCANCODE && scancodeElement <= eInputElements::KB_LAST_SCANCODE);

    return static_cast<eInputElements>(scancodeElement - eInputElements::KB_COUNT_VIRTUAL);
}

eInputElements KeyboardDeviceImpl::ConvertDavaVirtualToDavaScancode(eInputElements virtualElement)
{
    DVASSERT(virtualElement >= eInputElements::KB_FIRST_VIRTUAL && virtualElement <= eInputElements::KB_LAST_VIRTUAL);

    return static_cast<eInputElements>(virtualElement + eInputElements::KB_COUNT_SCANCODE);
}

} // namespace Private
} // namespace DAVA

#endif
