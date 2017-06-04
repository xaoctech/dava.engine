#include "Input/InputElements.h"

namespace DAVA
{
const InputElementInfo& GetInputElementInfo(eInputElements element)
{
    // Use multiple arrays to avoid using extra space for reserved ranges
    static InputElementInfo infoKeyboard[eInputElements::KB_LAST - eInputElements::KB_FIRST + 1];
    static InputElementInfo infoMouse[eInputElements::MOUSE_LAST - eInputElements::MOUSE_FIRST + 1];
    static InputElementInfo infoGamepad[eInputElements::GAMEPAD_LAST - eInputElements::GAMEPAD_FIRST + 1];
    static InputElementInfo infoTouch[eInputElements::TOUCH_LAST - eInputElements::TOUCH_FIRST + 1];
    static bool initialized = false;

    if (!initialized)
    {
        // Keyboard

        infoKeyboard[eInputElements::KB_1 - eInputElements::KB_FIRST] = { "1", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_2 - eInputElements::KB_FIRST] = { "2", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_3 - eInputElements::KB_FIRST] = { "3", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_4 - eInputElements::KB_FIRST] = { "4", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_5 - eInputElements::KB_FIRST] = { "5", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_6 - eInputElements::KB_FIRST] = { "6", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_7 - eInputElements::KB_FIRST] = { "7", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_8 - eInputElements::KB_FIRST] = { "8", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_9 - eInputElements::KB_FIRST] = { "9", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_0 - eInputElements::KB_FIRST] = { "0", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_A - eInputElements::KB_FIRST] = { "A", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_B - eInputElements::KB_FIRST] = { "B", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_C - eInputElements::KB_FIRST] = { "C", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_D - eInputElements::KB_FIRST] = { "D", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_E - eInputElements::KB_FIRST] = { "E", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F - eInputElements::KB_FIRST] = { "F", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_G - eInputElements::KB_FIRST] = { "G", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_H - eInputElements::KB_FIRST] = { "H", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_I - eInputElements::KB_FIRST] = { "I", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_J - eInputElements::KB_FIRST] = { "J", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_K - eInputElements::KB_FIRST] = { "K", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_L - eInputElements::KB_FIRST] = { "L", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_M - eInputElements::KB_FIRST] = { "M", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_N - eInputElements::KB_FIRST] = { "N", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_O - eInputElements::KB_FIRST] = { "O", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_P - eInputElements::KB_FIRST] = { "P", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_Q - eInputElements::KB_FIRST] = { "Q", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_R - eInputElements::KB_FIRST] = { "R", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_S - eInputElements::KB_FIRST] = { "S", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_T - eInputElements::KB_FIRST] = { "T", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_U - eInputElements::KB_FIRST] = { "U", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_V - eInputElements::KB_FIRST] = { "V", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_W - eInputElements::KB_FIRST] = { "W", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_X - eInputElements::KB_FIRST] = { "X", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_Y - eInputElements::KB_FIRST] = { "Y", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_Z - eInputElements::KB_FIRST] = { "Z", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F1 - eInputElements::KB_FIRST] = { "F1", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F2 - eInputElements::KB_FIRST] = { "F2", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F3 - eInputElements::KB_FIRST] = { "F3", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F4 - eInputElements::KB_FIRST] = { "F4", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F5 - eInputElements::KB_FIRST] = { "F5", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F6 - eInputElements::KB_FIRST] = { "F6", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F7 - eInputElements::KB_FIRST] = { "F7", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F8 - eInputElements::KB_FIRST] = { "F8", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F9 - eInputElements::KB_FIRST] = { "F9", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F10 - eInputElements::KB_FIRST] = { "F10", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F11 - eInputElements::KB_FIRST] = { "F11", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_F12 - eInputElements::KB_FIRST] = { "F12", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NONUSBACKSLASH - eInputElements::KB_FIRST] = { "\\", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_COMMA - eInputElements::KB_FIRST] = { ",", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_PERIOD - eInputElements::KB_FIRST] = { ".", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_SLASH - eInputElements::KB_FIRST] = { "/", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_BACKSLASH - eInputElements::KB_FIRST] = { "\\", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_APOSTROPHE - eInputElements::KB_FIRST] = { "'", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_SEMICOLON - eInputElements::KB_FIRST] = { ";", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RBRACKET - eInputElements::KB_FIRST] = { " - eInputElements::KB_FIRST]", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LBRACKET - eInputElements::KB_FIRST] = { "[", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_BACKSPACE - eInputElements::KB_FIRST] = { "Backspace", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_EQUALS - eInputElements::KB_FIRST] = { "=", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_MINUS - eInputElements::KB_FIRST] = { "-", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_ESCAPE - eInputElements::KB_FIRST] = { "Escape", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_GRAVE - eInputElements::KB_FIRST] = { "`", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_TAB - eInputElements::KB_FIRST] = { "Tab", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_CAPSLOCK - eInputElements::KB_FIRST] = { "Capslock", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LSHIFT - eInputElements::KB_FIRST] = { "LShift", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LCTRL - eInputElements::KB_FIRST] = { "LCtrl", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LWIN - eInputElements::KB_FIRST] = { "LWin", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LALT - eInputElements::KB_FIRST] = { "LAlt", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_SPACE - eInputElements::KB_FIRST] = { "Space", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RALT - eInputElements::KB_FIRST] = { "RAlt", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RWIN - eInputElements::KB_FIRST] = { "RWin", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_MENU - eInputElements::KB_FIRST] = { "Menu", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RCTRL - eInputElements::KB_FIRST] = { "RCtrl", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RSHIFT - eInputElements::KB_FIRST] = { "RShift", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_ENTER - eInputElements::KB_FIRST] = { "Enter", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_PRINTSCREEN - eInputElements::KB_FIRST] = { "PrintScreen", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_SCROLLLOCK - eInputElements::KB_FIRST] = { "Scrolllock", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_PAUSE - eInputElements::KB_FIRST] = { "Pause", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_INSERT - eInputElements::KB_FIRST] = { "Insert", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_HOME - eInputElements::KB_FIRST] = { "Home", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_PAGEUP - eInputElements::KB_FIRST] = { "Pageup", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_DELETE - eInputElements::KB_FIRST] = { "Del", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_END - eInputElements::KB_FIRST] = { "End", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_PAGEDOWN - eInputElements::KB_FIRST] = { "Pagedown", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_UP - eInputElements::KB_FIRST] = { "Up", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LEFT - eInputElements::KB_FIRST] = { "Left", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_DOWN - eInputElements::KB_FIRST] = { "Down", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RIGHT - eInputElements::KB_FIRST] = { "Right", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMLOCK - eInputElements::KB_FIRST] = { "Numlock", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_DIVIDE - eInputElements::KB_FIRST] = { "/", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_MULTIPLY - eInputElements::KB_FIRST] = { "*", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_MINUS - eInputElements::KB_FIRST] = { "-", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_PLUS - eInputElements::KB_FIRST] = { "+", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_ENTER - eInputElements::KB_FIRST] = { "Enter (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_DELETE - eInputElements::KB_FIRST] = { "Del (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_1 - eInputElements::KB_FIRST] = { "1 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_2 - eInputElements::KB_FIRST] = { "2 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_3 - eInputElements::KB_FIRST] = { "3 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_4 - eInputElements::KB_FIRST] = { "4 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_5 - eInputElements::KB_FIRST] = { "5 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_6 - eInputElements::KB_FIRST] = { "6 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_7 - eInputElements::KB_FIRST] = { "7 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_8 - eInputElements::KB_FIRST] = { "8 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_9 - eInputElements::KB_FIRST] = { "9 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_NUMPAD_0 - eInputElements::KB_FIRST] = { "10 (Numpad)", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_LCMD - eInputElements::KB_FIRST] = { "LCmd", eInputElementTypes::DIGITAL };
        infoKeyboard[eInputElements::KB_RCMD - eInputElements::KB_FIRST] = { "RCmd", eInputElementTypes::DIGITAL };

        // Mouse

        infoMouse[eInputElements::MOUSE_LBUTTON - eInputElements::MOUSE_FIRST] = { "Mouse left button", eInputElementTypes::DIGITAL };
        infoMouse[eInputElements::MOUSE_RBUTTON - eInputElements::MOUSE_FIRST] = { "Mouse right button", eInputElementTypes::DIGITAL };
        infoMouse[eInputElements::MOUSE_MBUTTON - eInputElements::MOUSE_FIRST] = { "Mouse middle button", eInputElementTypes::DIGITAL };
        infoMouse[eInputElements::MOUSE_EXT1BUTTON - eInputElements::MOUSE_FIRST] = { "Mouse extended button 1", eInputElementTypes::DIGITAL };
        infoMouse[eInputElements::MOUSE_EXT2BUTTON - eInputElements::MOUSE_FIRST] = { "Mouse extended button 2", eInputElementTypes::DIGITAL };
        infoMouse[eInputElements::MOUSE_WHEEL - eInputElements::MOUSE_FIRST] = { "Mouse wheel", eInputElementTypes::ANALOG };
        infoMouse[eInputElements::MOUSE_POSITION - eInputElements::MOUSE_FIRST] = { "Mouse cursor", eInputElementTypes::ANALOG };

        // Touch

        infoTouch[eInputElements::TOUCH_CLICK0 - eInputElements::TOUCH_FIRST] = { "Touch 0 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK1 - eInputElements::TOUCH_FIRST] = { "Touch 1 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK2 - eInputElements::TOUCH_FIRST] = { "Touch 2 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK3 - eInputElements::TOUCH_FIRST] = { "Touch 3 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK4 - eInputElements::TOUCH_FIRST] = { "Touch 4 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK5 - eInputElements::TOUCH_FIRST] = { "Touch 5 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK6 - eInputElements::TOUCH_FIRST] = { "Touch 6 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK7 - eInputElements::TOUCH_FIRST] = { "Touch 7 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK8 - eInputElements::TOUCH_FIRST] = { "Touch 8 click", eInputElementTypes::DIGITAL };
        infoTouch[eInputElements::TOUCH_CLICK9 - eInputElements::TOUCH_FIRST] = { "Touch 9 click", eInputElementTypes::DIGITAL };

        infoTouch[eInputElements::TOUCH_POSITION0 - eInputElements::TOUCH_FIRST] = { "Touch 0 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION1 - eInputElements::TOUCH_FIRST] = { "Touch 1 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION2 - eInputElements::TOUCH_FIRST] = { "Touch 2 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION3 - eInputElements::TOUCH_FIRST] = { "Touch 3 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION4 - eInputElements::TOUCH_FIRST] = { "Touch 4 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION5 - eInputElements::TOUCH_FIRST] = { "Touch 5 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION6 - eInputElements::TOUCH_FIRST] = { "Touch 6 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION7 - eInputElements::TOUCH_FIRST] = { "Touch 7 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION8 - eInputElements::TOUCH_FIRST] = { "Touch 8 position", eInputElementTypes::ANALOG };
        infoTouch[eInputElements::TOUCH_POSITION9 - eInputElements::TOUCH_FIRST] = { "Touch 9 position", eInputElementTypes::ANALOG };

        // Gamepad
        infoGamepad[eInputElements::GAMEPAD_START - eInputElements::GAMEPAD_FIRST] = { "Gamepad start", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_A - eInputElements::GAMEPAD_FIRST] = { "Gamepad A", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_B - eInputElements::GAMEPAD_FIRST] = { "Gamepad B", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_X - eInputElements::GAMEPAD_FIRST] = { "Gamepad X", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_Y - eInputElements::GAMEPAD_FIRST] = { "Gamepad Y", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_DPAD_LEFT - eInputElements::GAMEPAD_FIRST] = { "Gamepad dpad left", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_DPAD_RIGHT - eInputElements::GAMEPAD_FIRST] = { "Gamepad dpad right", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_DPAD_UP - eInputElements::GAMEPAD_FIRST] = { "Gamepad dpad up", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_DPAD_DOWN - eInputElements::GAMEPAD_FIRST] = { "Gamepad dpad down", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_LTHUMB - eInputElements::GAMEPAD_FIRST] = { "Gamepad left thumbstick", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_RTHUMB - eInputElements::GAMEPAD_FIRST] = { "Gamepad right thumbstick", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_LSHOULDER - eInputElements::GAMEPAD_FIRST] = { "Gamepad lelft shoulder", eInputElementTypes::DIGITAL };
        infoGamepad[eInputElements::GAMEPAD_RSHOULDER - eInputElements::GAMEPAD_FIRST] = { "Gamepad right shoulder", eInputElementTypes::DIGITAL };

        infoGamepad[eInputElements::GAMEPAD_AXIS_LTRIGGER - eInputElements::GAMEPAD_FIRST] = { "Gamepad left trigger", eInputElementTypes::ANALOG };
        infoGamepad[eInputElements::GAMEPAD_AXIS_RTRIGGER - eInputElements::GAMEPAD_FIRST] = { "Gamepad right trigger", eInputElementTypes::ANALOG };
        infoGamepad[eInputElements::GAMEPAD_AXIS_LTHUMB - eInputElements::GAMEPAD_FIRST] = { "Gamepad left thumbstick axis", eInputElementTypes::ANALOG };
        infoGamepad[eInputElements::GAMEPAD_AXIS_RTHUMB - eInputElements::GAMEPAD_FIRST] = { "Gamepad right thumbstick axis", eInputElementTypes::ANALOG };

        initialized = true;
    }

    if (IsKeyboardInputElement(element))
    {
        return infoKeyboard[element - eInputElements::KB_FIRST];
    }
    else if (IsMouseInputElement(element))
    {
        return infoMouse[element - eInputElements::MOUSE_FIRST];
    }
    else if (IsGamepadInputElement(element))
    {
        return infoGamepad[element - eInputElements::GAMEPAD_FIRST];
    }
    else if (IsTouchInputElement(element))
    {
        return infoTouch[element - eInputElements::TOUCH_FIRST];
    }
    else
    {
        DVASSERT(false);
        static InputElementInfo emptyInfo;
        return emptyInfo;
    }
}
}
