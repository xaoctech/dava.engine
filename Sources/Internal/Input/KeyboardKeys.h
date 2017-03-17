#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{

/**
    \ingroup input
    Keyboard keys.
    These values are used as a platform-independent representation of the keys.
*/
enum class eKeyboardKey : uint32
{
    UNKNOWN = 0,
    ESCAPE,
    BACKSPACE,
    TAB,
    ENTER,
    SPACE,

    LSHIFT,
    LCTRL,
    LALT,

    LCMD, // on mac - left cmd button
    RCMD, // on mac - right cmd button
    APPS, // https://en.wikipedia.org/wiki/Menu_key

    PAUSE,
    CAPSLOCK,
    NUMLOCK,
    SCROLLLOCK,

    PGUP,
    PGDN,
    HOME,
    END,
    INSERT,
    DELETE_, // TODO

    LEFT,
    UP,
    RIGHT,
    DOWN,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    GRAVE,
    MINUS,
    EQUALS,
    BACKSLASH,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    APOSTROPHE,
    COMMA,
    PERIOD,
    SLASH,

    NUMPAD0,
    NUMPAD1,
    NUMPAD2,
    NUMPAD3,
    NUMPAD4,
    NUMPAD5,
    NUMPAD6,
    NUMPAD7,
    NUMPAD8,
    NUMPAD9,

    MULTIPLY,
    DIVIDE,
    ADD,
    SUBTRACT,
    DECIMAL,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    BACK, // Android key
    MENU, // Android key

    NONUSBACKSLASH, // exist on some keyboards
    NUMPADENTER,
    PRINTSCREEN,
    RSHIFT,
    RCTRL,
    RALT,

    F13, // on mac - printscreen
    F14, // on mac - scrlock
    F15, // on mac - pause/break
    F16,
    F17,
    F18,
    F19,

    TOTAL_KEYS_COUNT
};

/**
    \ingroup input
    Translates platform-dependent key code to `eKeyboardKey` value;
*/
eKeyboardKey SystemKeyToDavaKey(uint32 systemKeyCode);

}