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
        info[eInputElements::KB_1_VIRTUAL] = { "1", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_2_VIRTUAL] = { "2", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_3_VIRTUAL] = { "3", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_4_VIRTUAL] = { "4", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_5_VIRTUAL] = { "5", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_6_VIRTUAL] = { "6", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_7_VIRTUAL] = { "7", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_8_VIRTUAL] = { "8", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_9_VIRTUAL] = { "9", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_0_VIRTUAL] = { "0", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_A_VIRTUAL] = { "A", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_B_VIRTUAL] = { "B", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_C_VIRTUAL] = { "C", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_D_VIRTUAL] = { "D", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_E_VIRTUAL] = { "E", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F_VIRTUAL] = { "F", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_G_VIRTUAL] = { "G", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_H_VIRTUAL] = { "H", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_I_VIRTUAL] = { "I", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_J_VIRTUAL] = { "J", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_K_VIRTUAL] = { "K", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_L_VIRTUAL] = { "L", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_M_VIRTUAL] = { "M", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_N_VIRTUAL] = { "N", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_O_VIRTUAL] = { "O", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_P_VIRTUAL] = { "P", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Q_VIRTUAL] = { "Q", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_R_VIRTUAL] = { "R", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_S_VIRTUAL] = { "S", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_T_VIRTUAL] = { "T", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_U_VIRTUAL] = { "U", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_V_VIRTUAL] = { "V", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_W_VIRTUAL] = { "W", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_X_VIRTUAL] = { "X", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Y_VIRTUAL] = { "Y", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Z_VIRTUAL] = { "Z", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F1_VIRTUAL] = { "F1", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F2_VIRTUAL] = { "F2", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F3_VIRTUAL] = { "F3", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F4_VIRTUAL] = { "F4", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F5_VIRTUAL] = { "F5", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F6_VIRTUAL] = { "F6", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F7_VIRTUAL] = { "F7", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F8_VIRTUAL] = { "F8", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F9_VIRTUAL] = { "F9", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F10_VIRTUAL] = { "F10", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F11_VIRTUAL] = { "F11", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F12_VIRTUAL] = { "F12", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NONUSBACKSLASH_VIRTUAL] = { "\\", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_COMMA_VIRTUAL] = { ",", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PERIOD_VIRTUAL] = { ".", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SLASH_VIRTUAL] = { "/", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSLASH_VIRTUAL] = { "\\", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_APOSTROPHE_VIRTUAL] = { "'", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SEMICOLON_VIRTUAL] = { ";", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RBRACKET_VIRTUAL] = { "]", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LBRACKET_VIRTUAL] = { "[", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSPACE_VIRTUAL] = { "Backspace", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_EQUALS_VIRTUAL] = { "=", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MINUS_VIRTUAL] = { "-", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ESCAPE_VIRTUAL] = { "Escape", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_GRAVE_VIRTUAL] = { "`", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_TAB_VIRTUAL] = { "Tab", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_CAPSLOCK_VIRTUAL] = { "Capslock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LSHIFT_VIRTUAL] = { "LShift", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCTRL_VIRTUAL] = { "LCtrl", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LWIN_VIRTUAL] = { "LWin", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LALT_VIRTUAL] = { "LAlt", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SPACE_VIRTUAL] = { "Space", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RALT_VIRTUAL] = { "RAlt", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RWIN_VIRTUAL] = { "RWin", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MENU_VIRTUAL] = { "Menu", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCTRL_VIRTUAL] = { "RCtrl", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RSHIFT_VIRTUAL] = { "RShift", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ENTER_VIRTUAL] = { "Enter", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PRINTSCREEN_VIRTUAL] = { "PrintScreen", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SCROLLLOCK_VIRTUAL] = { "Scrolllock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAUSE_VIRTUAL] = { "Pause", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_INSERT_VIRTUAL] = { "Insert", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_HOME_VIRTUAL] = { "Home", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEUP_VIRTUAL] = { "Pageup", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DELETE_VIRTUAL] = { "Del", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_END_VIRTUAL] = { "End", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEDOWN_VIRTUAL] = { "Pagedown", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_UP_VIRTUAL] = { "Up", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LEFT_VIRTUAL] = { "Left", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DOWN_VIRTUAL] = { "Down", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RIGHT_VIRTUAL] = { "Right", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMLOCK_VIRTUAL] = { "Numlock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DIVIDE_VIRTUAL] = { "/", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MULTIPLY_VIRTUAL] = { "*", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_MINUS_VIRTUAL] = { "-", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_PLUS_VIRTUAL] = { "+", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_ENTER_VIRTUAL] = { "Enter (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_DELETE_VIRTUAL] = { "Del (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_1_VIRTUAL] = { "1 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_2_VIRTUAL] = { "2 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_3_VIRTUAL] = { "3 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_4_VIRTUAL] = { "4 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_5_VIRTUAL] = { "5 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_6_VIRTUAL] = { "6 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_7_VIRTUAL] = { "7 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_8_VIRTUAL] = { "8 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_9_VIRTUAL] = { "9 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_0_VIRTUAL] = { "10 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCMD_VIRTUAL] = { "LCmd", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCMD_VIRTUAL] = { "RCmd", eInputElementTypes::DIGITAL };

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