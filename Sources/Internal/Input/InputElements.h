#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    List of all supported input elements.
    An input element is a part of a device which can be used for input. For example, a keyboard button, a mouse button, a mouse wheel, gamepad's stick etc.
*/
enum eInputElements : uint32
{
    NONE = 0,

    // Keyboard virtual keys

    KB_1_VIRTUAL,
    KB_2_VIRTUAL,
    KB_3_VIRTUAL,
    KB_4_VIRTUAL,
    KB_5_VIRTUAL,
    KB_6_VIRTUAL,
    KB_7_VIRTUAL,
    KB_8_VIRTUAL,
    KB_9_VIRTUAL,
    KB_0_VIRTUAL,
    KB_A_VIRTUAL,
    KB_B_VIRTUAL,
    KB_C_VIRTUAL,
    KB_D_VIRTUAL,
    KB_E_VIRTUAL,
    KB_F_VIRTUAL,
    KB_G_VIRTUAL,
    KB_H_VIRTUAL,
    KB_I_VIRTUAL,
    KB_J_VIRTUAL,
    KB_K_VIRTUAL,
    KB_L_VIRTUAL,
    KB_M_VIRTUAL,
    KB_N_VIRTUAL,
    KB_O_VIRTUAL,
    KB_P_VIRTUAL,
    KB_Q_VIRTUAL,
    KB_R_VIRTUAL,
    KB_S_VIRTUAL,
    KB_T_VIRTUAL,
    KB_U_VIRTUAL,
    KB_V_VIRTUAL,
    KB_W_VIRTUAL,
    KB_X_VIRTUAL,
    KB_Y_VIRTUAL,
    KB_Z_VIRTUAL,
    KB_F1_VIRTUAL,
    KB_F2_VIRTUAL,
    KB_F3_VIRTUAL,
    KB_F4_VIRTUAL,
    KB_F5_VIRTUAL,
    KB_F6_VIRTUAL,
    KB_F7_VIRTUAL,
    KB_F8_VIRTUAL,
    KB_F9_VIRTUAL,
    KB_F10_VIRTUAL,
    KB_F11_VIRTUAL,
    KB_F12_VIRTUAL,
    KB_NONUSBACKSLASH_VIRTUAL,
    KB_COMMA_VIRTUAL,
    KB_PERIOD_VIRTUAL,
    KB_SLASH_VIRTUAL,
    KB_BACKSLASH_VIRTUAL,
    KB_APOSTROPHE_VIRTUAL,
    KB_SEMICOLON_VIRTUAL,
    KB_RBRACKET_VIRTUAL,
    KB_LBRACKET_VIRTUAL,
    KB_BACKSPACE_VIRTUAL,
    KB_EQUALS_VIRTUAL,
    KB_MINUS_VIRTUAL,
    KB_ESCAPE_VIRTUAL,
    KB_GRAVE_VIRTUAL,
    KB_TAB_VIRTUAL,
    KB_CAPSLOCK_VIRTUAL,
    KB_LSHIFT_VIRTUAL,
    KB_LCTRL_VIRTUAL,
    KB_LWIN_VIRTUAL,
    KB_LALT_VIRTUAL,
    KB_SPACE_VIRTUAL,
    KB_RALT_VIRTUAL,
    KB_RWIN_VIRTUAL,
    KB_MENU_VIRTUAL,
    KB_RCTRL_VIRTUAL,
    KB_RSHIFT_VIRTUAL,
    KB_ENTER_VIRTUAL,
    KB_PRINTSCREEN_VIRTUAL,
    KB_SCROLLLOCK_VIRTUAL,
    KB_PAUSE_VIRTUAL,
    KB_INSERT_VIRTUAL,
    KB_HOME_VIRTUAL,
    KB_PAGEUP_VIRTUAL,
    KB_DELETE_VIRTUAL,
    KB_END_VIRTUAL,
    KB_PAGEDOWN_VIRTUAL,
    KB_UP_VIRTUAL,
    KB_LEFT_VIRTUAL,
    KB_DOWN_VIRTUAL,
    KB_RIGHT_VIRTUAL,
    KB_NUMLOCK_VIRTUAL,
    KB_DIVIDE_VIRTUAL,
    KB_MULTIPLY_VIRTUAL,
    KB_NUMPAD_MINUS_VIRTUAL,
    KB_NUMPAD_PLUS_VIRTUAL,
    KB_NUMPAD_ENTER_VIRTUAL,
    KB_NUMPAD_DELETE_VIRTUAL,
    KB_NUMPAD_1_VIRTUAL,
    KB_NUMPAD_2_VIRTUAL,
    KB_NUMPAD_3_VIRTUAL,
    KB_NUMPAD_4_VIRTUAL,
    KB_NUMPAD_5_VIRTUAL,
    KB_NUMPAD_6_VIRTUAL,
    KB_NUMPAD_7_VIRTUAL,
    KB_NUMPAD_8_VIRTUAL,
    KB_NUMPAD_9_VIRTUAL,
    KB_NUMPAD_0_VIRTUAL,
    KB_LCMD_VIRTUAL,
    KB_RCMD_VIRTUAL,

    // Keyboard scancode keys

    KB_1_SCANCODE,
    KB_2_SCANCODE,
    KB_3_SCANCODE,
    KB_4_SCANCODE,
    KB_5_SCANCODE,
    KB_6_SCANCODE,
    KB_7_SCANCODE,
    KB_8_SCANCODE,
    KB_9_SCANCODE,
    KB_0_SCANCODE,
    KB_A_SCANCODE,
    KB_B_SCANCODE,
    KB_C_SCANCODE,
    KB_D_SCANCODE,
    KB_E_SCANCODE,
    KB_F_SCANCODE,
    KB_G_SCANCODE,
    KB_H_SCANCODE,
    KB_I_SCANCODE,
    KB_J_SCANCODE,
    KB_K_SCANCODE,
    KB_L_SCANCODE,
    KB_M_SCANCODE,
    KB_N_SCANCODE,
    KB_O_SCANCODE,
    KB_P_SCANCODE,
    KB_Q_SCANCODE,
    KB_R_SCANCODE,
    KB_S_SCANCODE,
    KB_T_SCANCODE,
    KB_U_SCANCODE,
    KB_V_SCANCODE,
    KB_W_SCANCODE,
    KB_X_SCANCODE,
    KB_Y_SCANCODE,
    KB_Z_SCANCODE,
    KB_F1_SCANCODE,
    KB_F2_SCANCODE,
    KB_F3_SCANCODE,
    KB_F4_SCANCODE,
    KB_F5_SCANCODE,
    KB_F6_SCANCODE,
    KB_F7_SCANCODE,
    KB_F8_SCANCODE,
    KB_F9_SCANCODE,
    KB_F10_SCANCODE,
    KB_F11_SCANCODE,
    KB_F12_SCANCODE,
    KB_NONUSBACKSLASH_SCANCODE,
    KB_COMMA_SCANCODE,
    KB_PERIOD_SCANCODE,
    KB_SLASH_SCANCODE,
    KB_BACKSLASH_SCANCODE,
    KB_APOSTROPHE_SCANCODE,
    KB_SEMICOLON_SCANCODE,
    KB_RBRACKET_SCANCODE,
    KB_LBRACKET_SCANCODE,
    KB_BACKSPACE_SCANCODE,
    KB_EQUALS_SCANCODE,
    KB_MINUS_SCANCODE,
    KB_ESCAPE_SCANCODE,
    KB_GRAVE_SCANCODE,
    KB_TAB_SCANCODE,
    KB_CAPSLOCK_SCANCODE,
    KB_LSHIFT_SCANCODE,
    KB_LCTRL_SCANCODE,
    KB_LWIN_SCANCODE,
    KB_LALT_SCANCODE,
    KB_SPACE_SCANCODE,
    KB_RALT_SCANCODE,
    KB_RWIN_SCANCODE,
    KB_MENU_SCANCODE,
    KB_RCTRL_SCANCODE,
    KB_RSHIFT_SCANCODE,
    KB_ENTER_SCANCODE,
    KB_PRINTSCREEN_SCANCODE,
    KB_SCROLLLOCK_SCANCODE,
    KB_PAUSE_SCANCODE,
    KB_INSERT_SCANCODE,
    KB_HOME_SCANCODE,
    KB_PAGEUP_SCANCODE,
    KB_DELETE_SCANCODE,
    KB_END_SCANCODE,
    KB_PAGEDOWN_SCANCODE,
    KB_UP_SCANCODE,
    KB_LEFT_SCANCODE,
    KB_DOWN_SCANCODE,
    KB_RIGHT_SCANCODE,
    KB_NUMLOCK_SCANCODE,
    KB_DIVIDE_SCANCODE,
    KB_MULTIPLY_SCANCODE,
    KB_NUMPAD_MINUS_SCANCODE,
    KB_NUMPAD_PLUS_SCANCODE,
    KB_NUMPAD_ENTER_SCANCODE,
    KB_NUMPAD_DELETE_SCANCODE,
    KB_NUMPAD_1_SCANCODE,
    KB_NUMPAD_2_SCANCODE,
    KB_NUMPAD_3_SCANCODE,
    KB_NUMPAD_4_SCANCODE,
    KB_NUMPAD_5_SCANCODE,
    KB_NUMPAD_6_SCANCODE,
    KB_NUMPAD_7_SCANCODE,
    KB_NUMPAD_8_SCANCODE,
    KB_NUMPAD_9_SCANCODE,
    KB_NUMPAD_0_SCANCODE,
    KB_LCMD_SCANCODE,
    KB_RCMD_SCANCODE,

    // Mouse

    MOUSE_LBUTTON,
    MOUSE_RBUTTON,
    MOUSE_MBUTTON,
    MOUSE_EXT1BUTTON,
    MOUSE_EXT2BUTTON,
    MOUSE_WHEEL,
    MOUSE_POSITION,

    // Counters

    FIRST = NONE,
    LAST = MOUSE_POSITION,
    COUNT = LAST - FIRST + 1,

    MOUSE_FIRST = MOUSE_LBUTTON,
    MOUSE_LAST = MOUSE_POSITION,
    MOUSE_FIRST_BUTTON = MOUSE_LBUTTON,
    MOUSE_LAST_BUTTON = MOUSE_EXT2BUTTON,

    KB_FIRST = KB_1_VIRTUAL,
    KB_LAST = KB_RCMD_SCANCODE,
    KB_COUNT = KB_LAST - KB_FIRST + 1,

    KB_FIRST_SCANCODE = KB_1_SCANCODE,
    KB_LAST_SCANCODE = KB_RCMD_SCANCODE,
    KB_COUNT_SCANCODE = KB_LAST_SCANCODE - KB_FIRST_SCANCODE + 1,

    KB_FIRST_VIRTUAL = KB_1_VIRTUAL,
    KB_LAST_VIRTUAL = KB_RCMD_VIRTUAL,
    KB_COUNT_VIRTUAL = KB_LAST_VIRTUAL - KB_FIRST_VIRTUAL + 1,
};

/** List of input element types. */
enum eInputElementTypes
{
    /** Basically, a button, which can just be pressed and released. */
    DIGITAL,

    /**
		Element whose state can only be described using multiple float values.
		For example, gamepad's stick position can be described using normalized x and y values.
	*/
    ANALOG
};

/** Contains additional information about an element. */
struct InputElementInfo final
{
    String name;
    eInputElementTypes type;
};

inline bool IsMouseInputElement(eInputElements element)
{
    return eInputElements::MOUSE_FIRST <= element && element <= eInputElements::MOUSE_LAST;
}

inline bool IsKeyboardInputElement(eInputElements element)
{
    return eInputElements::KB_FIRST <= element && element <= eInputElements::KB_LAST;
}

inline bool IsKeyboardModifierInputElement(eInputElements element)
{
    return (element == eInputElements::KB_LSHIFT_VIRTUAL ||
            element == eInputElements::KB_LCTRL_VIRTUAL ||
            element == eInputElements::KB_LALT_VIRTUAL ||
            element == eInputElements::KB_RSHIFT_VIRTUAL ||
            element == eInputElements::KB_RCTRL_VIRTUAL ||
            element == eInputElements::KB_RALT_VIRTUAL);
}

inline bool IsKeyboardSystemInputElement(eInputElements element)
{
    return (element == eInputElements::KB_ESCAPE_VIRTUAL ||
            element == eInputElements::KB_CAPSLOCK_VIRTUAL ||
            element == eInputElements::KB_LWIN_VIRTUAL ||
            element == eInputElements::KB_RWIN_VIRTUAL ||
            element == eInputElements::KB_LCMD_VIRTUAL ||
            element == eInputElements::KB_RCMD_VIRTUAL ||
            element == eInputElements::KB_PRINTSCREEN_VIRTUAL ||
            element == eInputElements::KB_SCROLLLOCK_VIRTUAL ||
            element == eInputElements::KB_PAUSE_VIRTUAL ||
            element == eInputElements::KB_INSERT_VIRTUAL ||
            element == eInputElements::KB_HOME_VIRTUAL ||
            element == eInputElements::KB_PAGEUP_VIRTUAL ||
            element == eInputElements::KB_PAGEDOWN_VIRTUAL ||
            element == eInputElements::KB_DELETE_VIRTUAL ||
            element == eInputElements::KB_END_VIRTUAL ||
            element == eInputElements::KB_PAGEDOWN_VIRTUAL ||
            element == eInputElements::KB_NUMLOCK_VIRTUAL ||
            element == eInputElements::KB_MENU_VIRTUAL);
}

/** Get additional information about an element */
const InputElementInfo& GetInputElementInfo(eInputElements element);
} // namespace DAVA
