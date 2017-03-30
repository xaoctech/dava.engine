#include "Input/InputElements.h"

namespace DAVA
{
const InputElementInfo& GetInputElementInfo(eInputElements element)
{
    static InputElementInfo info[eInputElements::COUNT];
    static bool initialized = false;

    if (!initialized)
    {
        // Keyboard
        info[eInputElements::KB_1_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_2_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_3_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_4_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_5_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_6_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_7_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_8_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_9_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_0_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_A_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_B_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_C_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_D_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_E_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_G_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_H_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_I_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_J_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_K_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_L_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_M_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_N_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_O_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_P_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Q_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_R_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_S_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_T_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_U_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_V_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_W_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_X_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Y_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Z_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F1_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F2_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F3_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F4_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F5_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F6_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F7_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F8_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F9_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F10_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F11_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F12_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NONUSBACKSLASH_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_COMMA_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PERIOD_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SLASH_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSLASH_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_APOSTROPHE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SEMICOLON_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RBRACKET_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LBRACKET_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSPACE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_EQUALS_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MINUS_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ESCAPE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_GRAVE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_TAB_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_CAPSLOCK_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LSHIFT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCTRL_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LWIN_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LALT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SPACE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RALT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RWIN_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MENU_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCTRL_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RSHIFT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ENTER_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PRINTSCREEN_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SCROLLLOCK_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAUSE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_INSERT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_HOME_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEUP_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DELETE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_END_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEDOWN_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_UP_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LEFT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DOWN_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RIGHT_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMLOCK_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DIVIDE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MULTIPLY_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_MINUS_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_PLUS_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_ENTER_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_DELETE_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_1_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_2_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_3_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_4_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_5_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_6_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_7_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_8_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_9_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_0_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCMD_SCANCODE] = { "", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCMD_SCANCODE] = { "", eInputElementTypes::DIGITAL };

        // Mouse
        info[eInputElements::MOUSE_LBUTTON] = { "Mouse left button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_RBUTTON] = { "Mouse right button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_MBUTTON] = { "Mouse middle button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_EXT1BUTTON] = { "Mouse extended button 1", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_EXT2BUTTON] = { "Mouse extended button 2", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_WHEEL] = { "Mouse wheel", eInputElementTypes::ANALOG };
        info[eInputElements::MOUSE_POSITION] = { "Mouse cursor", eInputElementTypes::ANALOG };

        initialized = true;
    }

    return info[element];
}
}