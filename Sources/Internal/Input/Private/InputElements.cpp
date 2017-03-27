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
        info[KB_ESCAPE] = { "Escape", eInputElementType::DIGITAL };
        info[KB_BACKSPACE] = { "Backspace", eInputElementType::DIGITAL };
        info[KB_TAB] = { "Tab", eInputElementType::DIGITAL };
        info[KB_ENTER] = { "Enter", eInputElementType::DIGITAL };
        info[KB_SPACE] = { "Space", eInputElementType::DIGITAL };
        info[KB_LSHIFT] = { "Left shift", eInputElementType::DIGITAL };
        info[KB_LCTRL] = { "Left ctrl", eInputElementType::DIGITAL };
        info[KB_LALT] = { "Left alt", eInputElementType::DIGITAL };
        info[KB_LCMD] = { "Left cmd", eInputElementType::DIGITAL };
        info[KB_RCMD] = { "Right cmd", eInputElementType::DIGITAL };
        info[KB_APPS] = { "Apps", eInputElementType::DIGITAL };
        info[KB_PAUSE] = { "Pause", eInputElementType::DIGITAL };
        info[KB_CAPSLOCK] = { "Capslock", eInputElementType::DIGITAL };
        info[KB_NUMLOCK] = { "Numlock", eInputElementType::DIGITAL };
        info[KB_SCROLLLOCK] = { "Scrollock", eInputElementType::DIGITAL };
        info[KB_PGUP] = { "Page up", eInputElementType::DIGITAL };
        info[KB_PGDN] = { "Page down", eInputElementType::DIGITAL };
        info[KB_HOME] = { "Home", eInputElementType::DIGITAL };
        info[KB_END] = { "End", eInputElementType::DIGITAL };
        info[KB_INSERT] = { "Insert", eInputElementType::DIGITAL };
        info[KB_DELETE] = { "Delete", eInputElementType::DIGITAL };
        info[KB_LEFT] = { "Left", eInputElementType::DIGITAL };
        info[KB_UP] = { "Up", eInputElementType::DIGITAL };
        info[KB_RIGHT] = { "Right", eInputElementType::DIGITAL };
        info[KB_DOWN] = { "Down", eInputElementType::DIGITAL };
        info[KB_KEY_0] = { "0", eInputElementType::DIGITAL };
        info[KB_KEY_1] = { "1", eInputElementType::DIGITAL };
        info[KB_KEY_2] = { "2", eInputElementType::DIGITAL };
        info[KB_KEY_3] = { "3", eInputElementType::DIGITAL };
        info[KB_KEY_4] = { "4", eInputElementType::DIGITAL };
        info[KB_KEY_5] = { "5", eInputElementType::DIGITAL };
        info[KB_KEY_6] = { "6", eInputElementType::DIGITAL };
        info[KB_KEY_7] = { "7", eInputElementType::DIGITAL };
        info[KB_KEY_8] = { "8", eInputElementType::DIGITAL };
        info[KB_KEY_9] = { "9", eInputElementType::DIGITAL };
        info[KB_KEY_A] = { "A", eInputElementType::DIGITAL };
        info[KB_KEY_B] = { "B", eInputElementType::DIGITAL };
        info[KB_KEY_C] = { "C", eInputElementType::DIGITAL };
        info[KB_KEY_D] = { "D", eInputElementType::DIGITAL };
        info[KB_KEY_E] = { "E", eInputElementType::DIGITAL };
        info[KB_KEY_F] = { "F", eInputElementType::DIGITAL };
        info[KB_KEY_G] = { "G", eInputElementType::DIGITAL };
        info[KB_KEY_H] = { "H", eInputElementType::DIGITAL };
        info[KB_KEY_I] = { "I", eInputElementType::DIGITAL };
        info[KB_KEY_J] = { "J", eInputElementType::DIGITAL };
        info[KB_KEY_K] = { "K", eInputElementType::DIGITAL };
        info[KB_KEY_L] = { "L", eInputElementType::DIGITAL };
        info[KB_KEY_M] = { "M", eInputElementType::DIGITAL };
        info[KB_KEY_N] = { "N", eInputElementType::DIGITAL };
        info[KB_KEY_O] = { "O", eInputElementType::DIGITAL };
        info[KB_KEY_P] = { "P", eInputElementType::DIGITAL };
        info[KB_KEY_Q] = { "Q", eInputElementType::DIGITAL };
        info[KB_KEY_R] = { "R", eInputElementType::DIGITAL };
        info[KB_KEY_S] = { "S", eInputElementType::DIGITAL };
        info[KB_KEY_T] = { "T", eInputElementType::DIGITAL };
        info[KB_KEY_U] = { "U", eInputElementType::DIGITAL };
        info[KB_KEY_V] = { "V", eInputElementType::DIGITAL };
        info[KB_KEY_W] = { "W", eInputElementType::DIGITAL };
        info[KB_KEY_X] = { "X", eInputElementType::DIGITAL };
        info[KB_KEY_Y] = { "Y", eInputElementType::DIGITAL };
        info[KB_KEY_Z] = { "Z", eInputElementType::DIGITAL };
        info[KB_GRAVE] = { "`", eInputElementType::DIGITAL };
        info[KB_MINUS] = { "-", eInputElementType::DIGITAL };
        info[KB_EQUALS] = { "=", eInputElementType::DIGITAL };
        info[KB_BACKSLASH] = { "\\", eInputElementType::DIGITAL };
        info[KB_LBRACKET] = { "[", eInputElementType::DIGITAL };
        info[KB_RBRACKET] = { "]", eInputElementType::DIGITAL };
        info[KB_SEMICOLON] = { ":", eInputElementType::DIGITAL };
        info[KB_APOSTROPHE] = { "\"", eInputElementType::DIGITAL };
        info[KB_COMMA] = { ",", eInputElementType::DIGITAL };
        info[KB_PERIOD] = { ".", eInputElementType::DIGITAL };
        info[KB_SLASH] = { "/", eInputElementType::DIGITAL };
        info[KB_NUMPAD0] = { "Numpad 0", eInputElementType::DIGITAL };
        info[KB_NUMPAD1] = { "Numpad 1", eInputElementType::DIGITAL };
        info[KB_NUMPAD2] = { "Numpad 2", eInputElementType::DIGITAL };
        info[KB_NUMPAD3] = { "Numpad 3", eInputElementType::DIGITAL };
        info[KB_NUMPAD4] = { "Numpad 4", eInputElementType::DIGITAL };
        info[KB_NUMPAD5] = { "Numpad 5", eInputElementType::DIGITAL };
        info[KB_NUMPAD6] = { "Numpad 6", eInputElementType::DIGITAL };
        info[KB_NUMPAD7] = { "Numpad 7", eInputElementType::DIGITAL };
        info[KB_NUMPAD8] = { "Numpad 8", eInputElementType::DIGITAL };
        info[KB_NUMPAD9] = { "Numpad 9", eInputElementType::DIGITAL };
        info[KB_MULTIPLY] = { "*", eInputElementType::DIGITAL };
        info[KB_DIVIDE] = { "/", eInputElementType::DIGITAL };
        info[KB_ADD] = { "+", eInputElementType::DIGITAL };
        info[KB_SUBTRACT] = { "-", eInputElementType::DIGITAL };
        info[KB_DECIMAL] = { ".", eInputElementType::DIGITAL };
        info[KB_F1] = { "F1", eInputElementType::DIGITAL };
        info[KB_F2] = { "F2", eInputElementType::DIGITAL };
        info[KB_F3] = { "F3", eInputElementType::DIGITAL };
        info[KB_F4] = { "F4", eInputElementType::DIGITAL };
        info[KB_F5] = { "F5", eInputElementType::DIGITAL };
        info[KB_F6] = { "F6", eInputElementType::DIGITAL };
        info[KB_F7] = { "F7", eInputElementType::DIGITAL };
        info[KB_F8] = { "F8", eInputElementType::DIGITAL };
        info[KB_F9] = { "F9", eInputElementType::DIGITAL };
        info[KB_F10] = { "F10", eInputElementType::DIGITAL };
        info[KB_F11] = { "F11", eInputElementType::DIGITAL };
        info[KB_F12] = { "F12", eInputElementType::DIGITAL };
        info[KB_BACK] = { "Back", eInputElementType::DIGITAL };
        info[KB_MENU] = { "Menu", eInputElementType::DIGITAL };
        info[KB_NONUSBACKSLASH] = { "\\", eInputElementType::DIGITAL };
        info[KB_NUMPADENTER] = { "Enter", eInputElementType::DIGITAL };
        info[KB_RSHIFT] = { "Right shift", eInputElementType::DIGITAL };
        info[KB_RCTRL] = { "Right ctrl", eInputElementType::DIGITAL };
        info[KB_RALT] = { "Right alt", eInputElementType::DIGITAL };
        info[KB_F13] = { "F13", eInputElementType::DIGITAL };
        info[KB_F14] = { "F14", eInputElementType::DIGITAL };
        info[KB_F15] = { "F15", eInputElementType::DIGITAL };
        info[KB_F16] = { "F16", eInputElementType::DIGITAL };
        info[KB_F17] = { "F17", eInputElementType::DIGITAL };
        info[KB_F18] = { "F18", eInputElementType::DIGITAL };
        info[KB_F19] = { "F19", eInputElementType::DIGITAL };

        // Mouse
        info[MOUSE_LBUTTON] = { "Mouse left button", eInputElementType::DIGITAL };
        info[MOUSE_RBUTTON] = { "Mouse right button", eInputElementType::DIGITAL };
        info[MOUSE_MBUTTON] = { "Mouse middle button", eInputElementType::DIGITAL };
        info[MOUSE_EXT1BUTTON] = { "Mouse extended button 1", eInputElementType::DIGITAL };
        info[MOUSE_EXT2BUTTON] = { "Mouse extended button 2", eInputElementType::DIGITAL };
        info[MOUSE_POSITION] = { "Mouse cursor", eInputElementType::ANALOG };

        initialized = true;
    }

    return info[element];
}
}