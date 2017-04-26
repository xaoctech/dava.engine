#include "Input/InputElements.h"

namespace DAVA
{
const InputElementInfo& GetInputElementInfo(eInputElements element)
{
    static InputElementInfo info[eInputElements::LAST - eInputElements::FIRST + 1];
    static bool initialized = false;

    if (!initialized)
    {
        // Keyboard

        info[eInputElements::KB_1] = { "1", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_2] = { "2", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_3] = { "3", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_4] = { "4", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_5] = { "5", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_6] = { "6", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_7] = { "7", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_8] = { "8", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_9] = { "9", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_0] = { "0", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_A] = { "A", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_B] = { "B", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_C] = { "C", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_D] = { "D", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_E] = { "E", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F] = { "F", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_G] = { "G", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_H] = { "H", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_I] = { "I", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_J] = { "J", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_K] = { "K", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_L] = { "L", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_M] = { "M", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_N] = { "N", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_O] = { "O", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_P] = { "P", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Q] = { "Q", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_R] = { "R", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_S] = { "S", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_T] = { "T", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_U] = { "U", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_V] = { "V", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_W] = { "W", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_X] = { "X", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Y] = { "Y", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_Z] = { "Z", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F1] = { "F1", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F2] = { "F2", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F3] = { "F3", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F4] = { "F4", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F5] = { "F5", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F6] = { "F6", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F7] = { "F7", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F8] = { "F8", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F9] = { "F9", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F10] = { "F10", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F11] = { "F11", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_F12] = { "F12", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NONUSBACKSLASH] = { "\\", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_COMMA] = { ",", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PERIOD] = { ".", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SLASH] = { "/", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSLASH] = { "\\", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_APOSTROPHE] = { "'", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SEMICOLON] = { ";", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RBRACKET] = { "]", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LBRACKET] = { "[", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_BACKSPACE] = { "Backspace", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_EQUALS] = { "=", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MINUS] = { "-", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ESCAPE] = { "Escape", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_GRAVE] = { "`", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_TAB] = { "Tab", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_CAPSLOCK] = { "Capslock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LSHIFT] = { "LShift", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCTRL] = { "LCtrl", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LWIN] = { "LWin", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LALT] = { "LAlt", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SPACE] = { "Space", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RALT] = { "RAlt", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RWIN] = { "RWin", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MENU] = { "Menu", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCTRL] = { "RCtrl", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RSHIFT] = { "RShift", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_ENTER] = { "Enter", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PRINTSCREEN] = { "PrintScreen", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_SCROLLLOCK] = { "Scrolllock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAUSE] = { "Pause", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_INSERT] = { "Insert", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_HOME] = { "Home", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEUP] = { "Pageup", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DELETE] = { "Del", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_END] = { "End", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_PAGEDOWN] = { "Pagedown", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_UP] = { "Up", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LEFT] = { "Left", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DOWN] = { "Down", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RIGHT] = { "Right", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMLOCK] = { "Numlock", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_DIVIDE] = { "/", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_MULTIPLY] = { "*", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_MINUS] = { "-", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_PLUS] = { "+", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_ENTER] = { "Enter (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_DELETE] = { "Del (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_1] = { "1 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_2] = { "2 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_3] = { "3 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_4] = { "4 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_5] = { "5 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_6] = { "6 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_7] = { "7 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_8] = { "8 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_9] = { "9 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_NUMPAD_0] = { "10 (Numpad)", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_LCMD] = { "LCmd", eInputElementTypes::DIGITAL };
        info[eInputElements::KB_RCMD] = { "RCmd", eInputElementTypes::DIGITAL };

        // Mouse

        info[eInputElements::MOUSE_LBUTTON] = { "Mouse left button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_RBUTTON] = { "Mouse right button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_MBUTTON] = { "Mouse middle button", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_EXT1BUTTON] = { "Mouse extended button 1", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_EXT2BUTTON] = { "Mouse extended button 2", eInputElementTypes::DIGITAL };
        info[eInputElements::MOUSE_WHEEL] = { "Mouse wheel", eInputElementTypes::ANALOG };
        info[eInputElements::MOUSE_POSITION] = { "Mouse cursor", eInputElementTypes::ANALOG };

        // Touch

        info[eInputElements::TOUCH_CLICK0] = { "Touch 0 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK1] = { "Touch 1 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK2] = { "Touch 2 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK3] = { "Touch 3 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK4] = { "Touch 4 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK5] = { "Touch 5 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK6] = { "Touch 6 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK7] = { "Touch 7 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK8] = { "Touch 8 click", eInputElementTypes::DIGITAL };
        info[eInputElements::TOUCH_CLICK9] = { "Touch 9 click", eInputElementTypes::DIGITAL };

        info[eInputElements::TOUCH_POSITION0] = { "Touch 0 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION1] = { "Touch 1 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION2] = { "Touch 2 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION3] = { "Touch 3 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION4] = { "Touch 4 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION5] = { "Touch 5 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION6] = { "Touch 6 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION7] = { "Touch 7 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION8] = { "Touch 8 position", eInputElementTypes::ANALOG };
        info[eInputElements::TOUCH_POSITION9] = { "Touch 9 position", eInputElementTypes::ANALOG };

        // Gamepad
        info[eInputElements::GAMEPAD_START] = { "Gamepad start", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_A] = { "Gamepad A", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_B] = { "Gamepad B", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_X] = { "Gamepad X", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_Y] = { "Gamepad Y", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_DPAD_LEFT] = { "Gamepad dpad left", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_DPAD_RIGHT] = { "Gamepad dpad right", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_DPAD_UP] = { "Gamepad dpad up", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_DPAD_DOWN] = { "Gamepad dpad down", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_LTHUMB] = { "Gamepad left thumbstick", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_RTHUMB] = { "Gamepad right thumbstick", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_LSHOULDER] = { "Gamepad lelft shoulder", eInputElementTypes::DIGITAL };
        info[eInputElements::GAMEPAD_RSHOULDER] = { "Gamepad right shoulder", eInputElementTypes::DIGITAL };

        info[eInputElements::GAMEPAD_AXIS_LTRIGGER] = { "Gamepad left trigger", eInputElementTypes::ANALOG };
        info[eInputElements::GAMEPAD_AXIS_RTRIGGER] = { "Gamepad right trigger", eInputElementTypes::ANALOG };
        info[eInputElements::GAMEPAD_AXIS_LTHUMB] = { "Gamepad left thumbstick axis", eInputElementTypes::ANALOG };
        info[eInputElements::GAMEPAD_AXIS_RTHUMB] = { "Gamepad right thumbstick axis", eInputElementTypes::ANALOG };

        initialized = true;
    }

    return info[element];
}
}
