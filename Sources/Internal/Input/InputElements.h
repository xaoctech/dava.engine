#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{

// Keyboard controls


enum eInputElements : uint32
{
    NONE = 0,

    // Keyboard

    KB_ESCAPE,
    KB_BACKSPACE,
    KB_TAB,
    KB_ENTER,
    KB_SPACE,
    KB_LSHIFT,
    KB_LCTRL,
    KB_LALT,
    KB_LCMD, // on mac - left cmd button
    KB_RCMD, // on mac - right cmd button
    KB_APPS, // https://en.wikipedia.org/wiki/Menu_key
    KB_PAUSE,
    KB_CAPSLOCK,
    KB_NUMLOCK,
    KB_SCROLLLOCK,
    KB_PGUP,
    KB_PGDN,
    KB_HOME,
    KB_END,
    KB_INSERT,
    KB_DELETE_, // TODO
    KB_LEFT,
    KB_UP,
    KB_RIGHT,
    KB_DOWN,
    KB_KEY_0,
    KB_KEY_1,
    KB_KEY_2,
    KB_KEY_3,
    KB_KEY_4,
    KB_KEY_5,
    KB_KEY_6,
    KB_KEY_7,
    KB_KEY_8,
    KB_KEY_9,
    KB_KEY_A,
    KB_KEY_B,
    KB_KEY_C,
    KB_KEY_D,
    KB_KEY_E,
    KB_KEY_F,
    KB_KEY_G,
    KB_KEY_H,
    KB_KEY_I,
    KB_KEY_J,
    KB_KEY_K,
    KB_KEY_L,
    KB_KEY_M,
    KB_KEY_N,
    KB_KEY_O,
    KB_KEY_P,
    KB_KEY_Q,
    KB_KEY_R,
    KB_KEY_S,
    KB_KEY_T,
    KB_KEY_U,
    KB_KEY_V,
    KB_KEY_W,
    KB_KEY_X,
    KB_KEY_Y,
    KB_KEY_Z,
    KB_GRAVE,
    KB_MINUS,
    KB_EQUALS,
    KB_BACKSLASH,
    KB_LBRACKET,
    KB_RBRACKET,
    KB_SEMICOLON,
    KB_APOSTROPHE,
    KB_COMMA,
    KB_PERIOD,
    KB_SLASH,
    KB_NUMPAD0,
    KB_NUMPAD1,
    KB_NUMPAD2,
    KB_NUMPAD3,
    KB_NUMPAD4,
    KB_NUMPAD5,
    KB_NUMPAD6,
    KB_NUMPAD7,
    KB_NUMPAD8,
    KB_NUMPAD9,
    KB_MULTIPLY,
    KB_DIVIDE,
    KB_ADD,
    KB_SUBTRACT,
    KB_DECIMAL,
    KB_F1,
    KB_F2,
    KB_F3,
    KB_F4,
    KB_F5,
    KB_F6,
    KB_F7,
    KB_F8,
    KB_F9,
    KB_F10,
    KB_F11,
    KB_F12,
    KB_BACK, // Android key
    KB_MENU, // Android key
    KB_NONUSBACKSLASH, // exist on some keyboards
    KB_NUMPADENTER,
    KB_PRINTSCREEN,
    KB_RSHIFT,
    KB_RCTRL,
    KB_RALT,
    KB_F13, // on mac - printscreen
    KB_F14, // on mac - scrlock
    KB_F15, // on mac - pause/break
    KB_F16,
    KB_F17,
    KB_F18,
    KB_F19,

    // Mouse
    
    MOUSE_LBUTTON,
    MOUSE_RBUTTON,
    MOUSE_MBUTTON,
    MOUSE_EXT1BUTTON,
    MOUSE_EXT2BUTTON,
    MOUSE_POSITION,

    // Counters

    KB_FIRST = KB_ESCAPE,
    KB_LAST = KB_F19,
    KB_COUNT = KB_LAST - KB_FIRST,

    MOUSE_FIRST = MOUSE_LBUTTON,
    MOUSE_LAST = MOUSE_POSITION,
    MOUSE_COUNT = MOUSE_LAST - MOUSE_FIRST
};
}