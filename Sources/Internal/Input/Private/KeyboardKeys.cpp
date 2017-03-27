#include "Input/KeyboardKeys.h"
#include "Debug/DVAssert.h"
#include <algorithm>

#if defined(__DAVAENGINE_WIN32__)
#include <Windows.h>
#endif

namespace DAVA
{
// Used for key translation from system to dava
static const uint32 MAX_TRANSLATOR_KEYS = 512;
static Array<eInputControl, MAX_TRANSLATOR_KEYS> systemToDavaKeyTranslator;

static void InitSystemToDavaKeyTranslator()
{
    static_assert(static_cast<size_t>(eInputControl::KB_COUNT) < MAX_TRANSLATOR_KEYS, "Check translator array size");

    std::uninitialized_fill(std::begin(systemToDavaKeyTranslator), std::end(systemToDavaKeyTranslator), eInputControl::NONE);

#if defined(__DAVAENGINE_WINDOWS__)
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx

    systemToDavaKeyTranslator[VK_LEFT] = eInputControl::KB_LEFT;
    systemToDavaKeyTranslator[VK_RIGHT] = eInputControl::KB_RIGHT;
    systemToDavaKeyTranslator[VK_UP] = eInputControl::KB_UP;
    systemToDavaKeyTranslator[VK_DOWN] = eInputControl::KB_DOWN;
    systemToDavaKeyTranslator[VK_DELETE] = eInputControl::KB_DELETE_;
    systemToDavaKeyTranslator[VK_ESCAPE] = eInputControl::KB_ESCAPE;
    systemToDavaKeyTranslator[VK_BACK] = eInputControl::KB_BACKSPACE;
    systemToDavaKeyTranslator[VK_RETURN] = eInputControl::KB_ENTER;

    systemToDavaKeyTranslator[256 + VK_LEFT] = eInputControl::KB_LEFT; // extended key
    systemToDavaKeyTranslator[256 + VK_RIGHT] = eInputControl::KB_RIGHT; // extended key
    systemToDavaKeyTranslator[256 + VK_UP] = eInputControl::KB_UP; // extended key
    systemToDavaKeyTranslator[256 + VK_DOWN] = eInputControl::KB_DOWN; // extended key
    systemToDavaKeyTranslator[256 + VK_DELETE] = eInputControl::KB_DELETE_; // extended key
    systemToDavaKeyTranslator[256 + VK_RETURN] = eInputControl::KB_NUMPADENTER; // extended key

    systemToDavaKeyTranslator[VK_CONTROL] = eInputControl::KB_LCTRL;
    systemToDavaKeyTranslator[VK_MENU] = eInputControl::KB_LALT;
    systemToDavaKeyTranslator[VK_SHIFT] = eInputControl::KB_LSHIFT;
    systemToDavaKeyTranslator[VK_APPS] = eInputControl::KB_APPS;

    systemToDavaKeyTranslator[256 + VK_CONTROL] = eInputControl::KB_RCTRL;
    systemToDavaKeyTranslator[256 + VK_MENU] = eInputControl::KB_RALT;
    systemToDavaKeyTranslator[256 + VK_SHIFT] = eInputControl::KB_RSHIFT;
    systemToDavaKeyTranslator[256 + VK_APPS] = eInputControl::KB_APPS; // win api mark this key as extended

    systemToDavaKeyTranslator[256 + VK_NUMLOCK] = eInputControl::KB_NUMLOCK;
    systemToDavaKeyTranslator[VK_CAPITAL] = eInputControl::KB_CAPSLOCK;
    systemToDavaKeyTranslator[VK_PAUSE] = eInputControl::KB_PAUSE;
    systemToDavaKeyTranslator[VK_SCROLL] = eInputControl::KB_SCROLLLOCK;
    systemToDavaKeyTranslator[256 + VK_SNAPSHOT] = eInputControl::KB_PRINTSCREEN;
    systemToDavaKeyTranslator[VK_SPACE] = eInputControl::KB_SPACE;
    systemToDavaKeyTranslator[VK_TAB] = eInputControl::KB_TAB;
    systemToDavaKeyTranslator[VK_ADD] = eInputControl::KB_ADD;
    systemToDavaKeyTranslator[VK_SUBTRACT] = eInputControl::KB_SUBTRACT;

    systemToDavaKeyTranslator[VK_HOME] = eInputControl::KB_HOME;
    systemToDavaKeyTranslator[VK_END] = eInputControl::KB_END;
    systemToDavaKeyTranslator[VK_PRIOR] = eInputControl::KB_PGUP;
    systemToDavaKeyTranslator[VK_NEXT] = eInputControl::KB_PGDN;
    systemToDavaKeyTranslator[VK_INSERT] = eInputControl::KB_INSERT;
    systemToDavaKeyTranslator[256 + VK_HOME] = eInputControl::KB_HOME; // extended key
    systemToDavaKeyTranslator[256 + VK_END] = eInputControl::KB_END; // extended key
    systemToDavaKeyTranslator[256 + VK_PRIOR] = eInputControl::KB_PGUP; // extended key
    systemToDavaKeyTranslator[256 + VK_NEXT] = eInputControl::KB_PGDN; // extended key
    systemToDavaKeyTranslator[256 + VK_INSERT] = eInputControl::KB_INSERT; // extended key

    systemToDavaKeyTranslator[VK_OEM_PLUS] = eInputControl::KB_EQUALS;
    systemToDavaKeyTranslator[VK_OEM_MINUS] = eInputControl::KB_MINUS;
    systemToDavaKeyTranslator[VK_OEM_PERIOD] = eInputControl::KB_PERIOD;
    systemToDavaKeyTranslator[VK_OEM_COMMA] = eInputControl::KB_COMMA;
    systemToDavaKeyTranslator[VK_OEM_1] = eInputControl::KB_SEMICOLON;
    systemToDavaKeyTranslator[VK_OEM_2] = eInputControl::KB_SLASH;
    systemToDavaKeyTranslator[VK_OEM_3] = eInputControl::KB_GRAVE;
    systemToDavaKeyTranslator[VK_OEM_4] = eInputControl::KB_LBRACKET;
    systemToDavaKeyTranslator[VK_OEM_5] = eInputControl::KB_BACKSLASH;
    systemToDavaKeyTranslator[VK_OEM_6] = eInputControl::KB_RBRACKET;
    systemToDavaKeyTranslator[VK_OEM_7] = eInputControl::KB_APOSTROPHE;

    systemToDavaKeyTranslator[VK_OEM_102] = eInputControl::KB_NONUSBACKSLASH;

    const unsigned numFuncKeys = static_cast<unsigned>(eInputControl::KB_F12) - static_cast<unsigned>(eInputControl::KB_F1);
    for (unsigned i = 0; i <= numFuncKeys; i++)
    {
        unsigned keyValue = static_cast<unsigned>(eInputControl::KB_F1) + i;
        systemToDavaKeyTranslator[VK_F1 + i] = static_cast<eInputControl>(keyValue);
    }

    // alpha keys
    for (unsigned i = 0; i < 26; ++i)
    {
        unsigned keyValue = static_cast<unsigned>(eInputControl::KB_KEY_A) + i;
        systemToDavaKeyTranslator[0x41 + i] = static_cast<eInputControl>(keyValue);
    }

    // numeric keys & keys at num pad
    for (unsigned i = 0; i < 10; ++i)
    {
        unsigned keyNum = static_cast<unsigned>(eInputControl::KB_KEY_0) + i;
        unsigned keyNumpad = static_cast<unsigned>(eInputControl::KB_NUMPAD0) + i;
        systemToDavaKeyTranslator[0x30 + i] = static_cast<eInputControl>(keyNum);
        systemToDavaKeyTranslator[0x60 + i] = static_cast<eInputControl>(keyNumpad);
    }
    systemToDavaKeyTranslator[VK_MULTIPLY] = eInputControl::KB_MULTIPLY;
    systemToDavaKeyTranslator[256 + VK_DIVIDE] = eInputControl::KB_DIVIDE; // extended key
    systemToDavaKeyTranslator[VK_DECIMAL] = eInputControl::KB_DECIMAL;
#endif

#if defined(__DAVAENGINE_MACOS__)
    systemToDavaKeyTranslator[0x7B] = eInputControl::KB_LEFT;
    systemToDavaKeyTranslator[0x7C] = eInputControl::KB_RIGHT;
    systemToDavaKeyTranslator[0x7E] = eInputControl::KB_UP;
    systemToDavaKeyTranslator[0x7D] = eInputControl::KB_DOWN;
    systemToDavaKeyTranslator[0x75] = eInputControl::KB_DELETE;
    systemToDavaKeyTranslator[0x35] = eInputControl::KB_ESCAPE;
    systemToDavaKeyTranslator[0x33] = eInputControl::KB_BACKSPACE;
    systemToDavaKeyTranslator[0x24] = eInputControl::KB_ENTER;
    systemToDavaKeyTranslator[0x30] = eInputControl::KB_TAB;

    systemToDavaKeyTranslator[59] = eInputControl::KB_LCTRL;
    systemToDavaKeyTranslator[58] = eInputControl::KB_LALT;
    systemToDavaKeyTranslator[56] = eInputControl::KB_LSHIFT;
    systemToDavaKeyTranslator[62] = eInputControl::KB_RCTRL;
    systemToDavaKeyTranslator[61] = eInputControl::KB_RALT;
    systemToDavaKeyTranslator[60] = eInputControl::KB_RSHIFT;

    systemToDavaKeyTranslator[57] = eInputControl::KB_CAPSLOCK;
    systemToDavaKeyTranslator[54] = eInputControl::KB_RCMD;
    systemToDavaKeyTranslator[55] = eInputControl::KB_LCMD; // LGUI in SDL
    systemToDavaKeyTranslator[0x31] = eInputControl::KB_SPACE;

    // from SDL2 scancodes_darwin.h
    systemToDavaKeyTranslator[10] = eInputControl::KB_NONUSBACKSLASH;
    systemToDavaKeyTranslator[24] = eInputControl::KB_EQUALS;
    systemToDavaKeyTranslator[27] = eInputControl::KB_MINUS;
    systemToDavaKeyTranslator[47] = eInputControl::KB_PERIOD;
    systemToDavaKeyTranslator[43] = eInputControl::KB_COMMA;
    systemToDavaKeyTranslator[41] = eInputControl::KB_SEMICOLON;
    systemToDavaKeyTranslator[44] = eInputControl::KB_SLASH;
    systemToDavaKeyTranslator[50] = eInputControl::KB_GRAVE;
    systemToDavaKeyTranslator[33] = eInputControl::KB_LBRACKET;
    systemToDavaKeyTranslator[42] = eInputControl::KB_BACKSLASH;
    systemToDavaKeyTranslator[30] = eInputControl::KB_RBRACKET;
    systemToDavaKeyTranslator[39] = eInputControl::KB_APOSTROPHE;
    systemToDavaKeyTranslator[114] = eInputControl::KB_INSERT;
    systemToDavaKeyTranslator[115] = eInputControl::KB_HOME;
    systemToDavaKeyTranslator[116] = eInputControl::KB_PGUP;
    systemToDavaKeyTranslator[119] = eInputControl::KB_END;
    systemToDavaKeyTranslator[121] = eInputControl::KB_PGDN;
    systemToDavaKeyTranslator[69] = eInputControl::KB_ADD;
    systemToDavaKeyTranslator[78] = eInputControl::KB_MINUS;
    systemToDavaKeyTranslator[67] = eInputControl::KB_MULTIPLY;
    systemToDavaKeyTranslator[75] = eInputControl::KB_DIVIDE;
    systemToDavaKeyTranslator[81] = eInputControl::KB_EQUALS;
    systemToDavaKeyTranslator[65] = eInputControl::KB_PERIOD;

    systemToDavaKeyTranslator[0x00] = eInputControl::KB_KEY_A;
    systemToDavaKeyTranslator[0x0B] = eInputControl::KB_KEY_B;
    systemToDavaKeyTranslator[0x08] = eInputControl::KB_KEY_C;
    systemToDavaKeyTranslator[0x02] = eInputControl::KB_KEY_D;
    systemToDavaKeyTranslator[0x0E] = eInputControl::KB_KEY_E;
    systemToDavaKeyTranslator[0x03] = eInputControl::KB_KEY_F;
    systemToDavaKeyTranslator[0x05] = eInputControl::KB_KEY_G;
    systemToDavaKeyTranslator[0x04] = eInputControl::KB_KEY_H;
    systemToDavaKeyTranslator[0x22] = eInputControl::KB_KEY_I;
    systemToDavaKeyTranslator[0x26] = eInputControl::KB_KEY_J;
    systemToDavaKeyTranslator[0x28] = eInputControl::KB_KEY_K;
    systemToDavaKeyTranslator[0x25] = eInputControl::KB_KEY_L;
    systemToDavaKeyTranslator[0x2D] = eInputControl::KB_KEY_N;
    systemToDavaKeyTranslator[0x2E] = eInputControl::KB_KEY_M;
    systemToDavaKeyTranslator[0x1F] = eInputControl::KB_KEY_O;
    systemToDavaKeyTranslator[0x23] = eInputControl::KB_KEY_P;
    systemToDavaKeyTranslator[0x0C] = eInputControl::KB_KEY_Q;
    systemToDavaKeyTranslator[0x0F] = eInputControl::KB_KEY_R;
    systemToDavaKeyTranslator[0x01] = eInputControl::KB_KEY_S;
    systemToDavaKeyTranslator[0x11] = eInputControl::KB_KEY_T;
    systemToDavaKeyTranslator[0x20] = eInputControl::KB_KEY_U;
    systemToDavaKeyTranslator[0x09] = eInputControl::KB_KEY_V;
    systemToDavaKeyTranslator[0x0D] = eInputControl::KB_KEY_W;
    systemToDavaKeyTranslator[0x07] = eInputControl::KB_KEY_X;
    systemToDavaKeyTranslator[0x10] = eInputControl::KB_KEY_Y;
    systemToDavaKeyTranslator[0x06] = eInputControl::KB_KEY_Z;

    systemToDavaKeyTranslator[0x1D] = eInputControl::KB_KEY_0;
    systemToDavaKeyTranslator[0x12] = eInputControl::KB_KEY_1;
    systemToDavaKeyTranslator[0x13] = eInputControl::KB_KEY_2;
    systemToDavaKeyTranslator[0x14] = eInputControl::KB_KEY_3;
    systemToDavaKeyTranslator[0x15] = eInputControl::KB_KEY_4;
    systemToDavaKeyTranslator[0x17] = eInputControl::KB_KEY_5;
    systemToDavaKeyTranslator[0x16] = eInputControl::KB_KEY_6;
    systemToDavaKeyTranslator[0x1A] = eInputControl::KB_KEY_7;
    systemToDavaKeyTranslator[0x1C] = eInputControl::KB_KEY_8;
    systemToDavaKeyTranslator[0x19] = eInputControl::KB_KEY_9;
    systemToDavaKeyTranslator[0x1B] = eInputControl::KB_MINUS;
    systemToDavaKeyTranslator[0x18] = eInputControl::KB_EQUALS;

    systemToDavaKeyTranslator[0x7A] = eInputControl::KB_F1;
    systemToDavaKeyTranslator[0x78] = eInputControl::KB_F2;
    systemToDavaKeyTranslator[0x76] = eInputControl::KB_F4;
    systemToDavaKeyTranslator[0x60] = eInputControl::KB_F5;
    systemToDavaKeyTranslator[0x61] = eInputControl::KB_F6;
    systemToDavaKeyTranslator[0x62] = eInputControl::KB_F7;
    systemToDavaKeyTranslator[0x63] = eInputControl::KB_F3;
    systemToDavaKeyTranslator[0x64] = eInputControl::KB_F8;
    systemToDavaKeyTranslator[0x65] = eInputControl::KB_F9;
    systemToDavaKeyTranslator[0x6D] = eInputControl::KB_F10;
    systemToDavaKeyTranslator[0x67] = eInputControl::KB_F11;
    systemToDavaKeyTranslator[0x6F] = eInputControl::KB_F12;

    systemToDavaKeyTranslator[113] = eInputControl::KB_F15;
    systemToDavaKeyTranslator[0x6A] = eInputControl::KB_F16;
    systemToDavaKeyTranslator[0x40] = eInputControl::KB_F17;
    systemToDavaKeyTranslator[0x4F] = eInputControl::KB_F18;
    systemToDavaKeyTranslator[0x50] = eInputControl::KB_F19;

    // numeric keys at numpad
    for (unsigned i = 0; i < 8; ++i)
    {
        systemToDavaKeyTranslator[0x52 + i] = static_cast<eKeyboardKey>(static_cast<unsigned>(eInputControl::KB_NUMPAD0) + i);
    }
    systemToDavaKeyTranslator[91] = eInputControl::KB_NUMPAD8;
    systemToDavaKeyTranslator[92] = eInputControl::KB_NUMPAD9;

    systemToDavaKeyTranslator[71] = eInputControl::KB_NUMLOCK;
    systemToDavaKeyTranslator[76] = eInputControl::KB_NUMPADENTER;
    systemToDavaKeyTranslator[65] = eInputControl::KB_DECIMAL;
    systemToDavaKeyTranslator[110] = eInputControl::KB_APPS;
    systemToDavaKeyTranslator[107] = eInputControl::KB_SCROLLLOCK;
    systemToDavaKeyTranslator[105] = eInputControl::KB_PRINTSCREEN;
#endif

#if defined(__DAVAENGINE_ANDROID__)
    systemToDavaKeyTranslator[0] = eInputControl::KB_UNKNOWN;
    systemToDavaKeyTranslator[1] = eInputControl::KB_LEFT;
    systemToDavaKeyTranslator[2] = eInputControl::KB_RIGHT;
    systemToDavaKeyTranslator[3] = eInputControl::KB_HOME;
    systemToDavaKeyTranslator[4] = eInputControl::KB_BACK;
    // systemToDavaKeyTranslator[5] = eInputControl::KB_CALL;
    // systemToDavaKeyTranslator[6] = eInputControl::KB_ENDCALL;
    systemToDavaKeyTranslator[7] = eInputControl::KB_KEY_0;
    systemToDavaKeyTranslator[8] = eInputControl::KB_KEY_1;
    systemToDavaKeyTranslator[9] = eInputControl::KB_KEY_2;
    systemToDavaKeyTranslator[10] = eInputControl::KB_KEY_3;
    systemToDavaKeyTranslator[11] = eInputControl::KB_KEY_4;
    systemToDavaKeyTranslator[12] = eInputControl::KB_KEY_5;
    systemToDavaKeyTranslator[13] = eInputControl::KB_KEY_6;
    systemToDavaKeyTranslator[14] = eInputControl::KB_KEY_7;
    systemToDavaKeyTranslator[15] = eInputControl::KB_KEY_8;
    systemToDavaKeyTranslator[16] = eInputControl::KB_KEY_9;
    // systemToDavaKeyTranslator[17] = eInputControl::KB_STAR;
    // systemToDavaKeyTranslator[18] = eInputControl::KB_POUND;
    // systemToDavaKeyTranslator[19] = eInputControl::KB_DPAD_UP;
    // systemToDavaKeyTranslator[20] = eInputControl::KB_DPAD_DOWN;
    // systemToDavaKeyTranslator[21] = eInputControl::KB_DPAD_LEFT;
    // systemToDavaKeyTranslator[22] = eInputControl::KB_DPAD_RIGHT;
    // systemToDavaKeyTranslator[23] = eInputControl::KB_DPAD_CENTER;
    // systemToDavaKeyTranslator[24] = eInputControl::KB_VOLUME_UP;
    // systemToDavaKeyTranslator[25] = eInputControl::KB_VOLUME_DOWN;
    // systemToDavaKeyTranslator[26] = eInputControl::KB_POWER;
    // systemToDavaKeyTranslator[27] = eInputControl::KB_CAMERA;
    // systemToDavaKeyTranslator[28] = eInputControl::KB_CLEAR;
    systemToDavaKeyTranslator[29] = eInputControl::KB_KEY_A;
    systemToDavaKeyTranslator[30] = eInputControl::KB_KEY_B;
    systemToDavaKeyTranslator[31] = eInputControl::KB_KEY_C;
    systemToDavaKeyTranslator[32] = eInputControl::KB_KEY_D;
    systemToDavaKeyTranslator[33] = eInputControl::KB_KEY_E;
    systemToDavaKeyTranslator[34] = eInputControl::KB_KEY_F;
    systemToDavaKeyTranslator[35] = eInputControl::KB_KEY_G;
    systemToDavaKeyTranslator[36] = eInputControl::KB_KEY_H;
    systemToDavaKeyTranslator[37] = eInputControl::KB_KEY_I;
    systemToDavaKeyTranslator[38] = eInputControl::KB_KEY_J;
    systemToDavaKeyTranslator[39] = eInputControl::KB_KEY_K;
    systemToDavaKeyTranslator[40] = eInputControl::KB_KEY_L;
    systemToDavaKeyTranslator[41] = eInputControl::KB_KEY_M;
    systemToDavaKeyTranslator[42] = eInputControl::KB_KEY_N;
    systemToDavaKeyTranslator[43] = eInputControl::KB_KEY_O;
    systemToDavaKeyTranslator[44] = eInputControl::KB_KEY_P;
    systemToDavaKeyTranslator[45] = eInputControl::KB_KEY_Q;
    systemToDavaKeyTranslator[46] = eInputControl::KB_KEY_R;
    systemToDavaKeyTranslator[47] = eInputControl::KB_KEY_S;
    systemToDavaKeyTranslator[48] = eInputControl::KB_KEY_T;
    systemToDavaKeyTranslator[49] = eInputControl::KB_KEY_U;
    systemToDavaKeyTranslator[50] = eInputControl::KB_KEY_V;
    systemToDavaKeyTranslator[51] = eInputControl::KB_KEY_W;
    systemToDavaKeyTranslator[52] = eInputControl::KB_KEY_X;
    systemToDavaKeyTranslator[53] = eInputControl::KB_KEY_Y;
    systemToDavaKeyTranslator[54] = eInputControl::KB_KEY_Z;
    systemToDavaKeyTranslator[55] = eInputControl::KB_COMMA;
    systemToDavaKeyTranslator[56] = eInputControl::KB_PERIOD;
    systemToDavaKeyTranslator[57] = eInputControl::KB_LALT;
    systemToDavaKeyTranslator[58] = eInputControl::KB_RALT;
    systemToDavaKeyTranslator[59] = eInputControl::KB_LSHIFT;
    systemToDavaKeyTranslator[60] = eInputControl::KB_RSHIFT;
    systemToDavaKeyTranslator[61] = eInputControl::KB_TAB;
    systemToDavaKeyTranslator[62] = eInputControl::KB_SPACE;
    // systemToDavaKeyTranslator[63] = eInputControl::KB_SYM;
    // systemToDavaKeyTranslator[64] = eInputControl::KB_EXPLORER;
    // systemToDavaKeyTranslator[65] = eInputControl::KB_ENVELOPE;
    systemToDavaKeyTranslator[66] = eInputControl::KB_ENTER;
    systemToDavaKeyTranslator[67] = eInputControl::KB_DELETE;
    systemToDavaKeyTranslator[68] = eInputControl::KB_GRAVE;
    systemToDavaKeyTranslator[69] = eInputControl::KB_MINUS;
    systemToDavaKeyTranslator[70] = eInputControl::KB_EQUALS;
    systemToDavaKeyTranslator[71] = eInputControl::KB_LBRACKET;
    systemToDavaKeyTranslator[72] = eInputControl::KB_RBRACKET;
    systemToDavaKeyTranslator[73] = eInputControl::KB_BACKSLASH;
    systemToDavaKeyTranslator[74] = eInputControl::KB_SEMICOLON;
    systemToDavaKeyTranslator[75] = eInputControl::KB_APOSTROPHE;
    systemToDavaKeyTranslator[76] = eInputControl::KB_SLASH;
    // systemToDavaKeyTranslator[77] = eInputControl::KB_AT;
    // systemToDavaKeyTranslator[78] = eInputControl::KB_NUM;
    // systemToDavaKeyTranslator[79] = eInputControl::KB_HEADSETHOOK;
    // systemToDavaKeyTranslator[80] = eInputControl::KB_FOCUS;
    // systemToDavaKeyTranslator[81] = eInputControl::KB_PLUS;
    systemToDavaKeyTranslator[82] = eInputControl::KB_MENU;
    // systemToDavaKeyTranslator[83] = eInputControl::KB_NOTIFICATION;
    // systemToDavaKeyTranslator[84] = eInputControl::KB_SEARCH;
    // systemToDavaKeyTranslator[85] = eInputControl::KB_MEDIA_PLAY_PAUSE;
    // systemToDavaKeyTranslator[86] = eInputControl::KB_MEDIA_STOP;
    // systemToDavaKeyTranslator[87] = eInputControl::KB_MEDIA_NEXT;
    // systemToDavaKeyTranslator[88] = eInputControl::KB_MEDIA_PREVIOUS;
    // systemToDavaKeyTranslator[89] = eInputControl::KB_MEDIA_REWIND;
    // systemToDavaKeyTranslator[90] = eInputControl::KB_MEDIA_FAST_FORWARD;
    // systemToDavaKeyTranslator[91] = eInputControl::KB_MUTE;
    systemToDavaKeyTranslator[92] = eInputControl::KB_PGUP;
    systemToDavaKeyTranslator[93] = eInputControl::KB_PGDN;
    // systemToDavaKeyTranslator[94] = eInputControl::KB_PICTSYMBOLS;
    // systemToDavaKeyTranslator[95] = eInputControl::KB_SWITCH_CHARSET;
    // systemToDavaKeyTranslator[96] = eInputControl::KB_BUTTON_A;
    // systemToDavaKeyTranslator[97] = eInputControl::KB_BUTTON_B;
    // systemToDavaKeyTranslator[98] = eInputControl::KB_BUTTON_C;
    // systemToDavaKeyTranslator[99] = eInputControl::KB_BUTTON_X;
    // systemToDavaKeyTranslator[100] = eInputControl::KB_BUTTON_Y;
    // systemToDavaKeyTranslator[101] = eInputControl::KB_BUTTON_Z;
    // systemToDavaKeyTranslator[102] = eInputControl::KB_BUTTON_L1;
    // systemToDavaKeyTranslator[103] = eInputControl::KB_BUTTON_R1;
    // systemToDavaKeyTranslator[104] = eInputControl::KB_BUTTON_L2;
    // systemToDavaKeyTranslator[105] = eInputControl::KB_BUTTON_R2;
    // systemToDavaKeyTranslator[106] = eInputControl::KB_BUTTON_THUMBL;
    // systemToDavaKeyTranslator[107] = eInputControl::KB_BUTTON_THUMBR;
    // systemToDavaKeyTranslator[108] = eInputControl::KB_BUTTON_START;
    // systemToDavaKeyTranslator[109] = eInputControl::KB_BUTTON_SELECT;
    // systemToDavaKeyTranslator[110] = eInputControl::KB_BUTTON_MODE;
    systemToDavaKeyTranslator[111] = eInputControl::KB_ESCAPE;
    // systemToDavaKeyTranslator[112] = eInputControl::KB_FORWARD_DEL;
    systemToDavaKeyTranslator[113] = eInputControl::KB_LCTRL;
    systemToDavaKeyTranslator[114] = eInputControl::KB_RCTRL;
    systemToDavaKeyTranslator[115] = eInputControl::KB_CAPSLOCK;
    systemToDavaKeyTranslator[116] = eInputControl::KB_SCROLLLOCK;
    systemToDavaKeyTranslator[117] = eInputControl::KB_LCMD;
    systemToDavaKeyTranslator[118] = eInputControl::KB_RCMD;
    // systemToDavaKeyTranslator[119] = eInputControl::KB_FUNCTION;
    // systemToDavaKeyTranslator[120] = eInputControl::KB_SYSRQ;
    systemToDavaKeyTranslator[121] = eInputControl::KB_PAUSE;
    systemToDavaKeyTranslator[122] = eInputControl::KB_HOME;
    systemToDavaKeyTranslator[123] = eInputControl::KB_END;
    systemToDavaKeyTranslator[124] = eInputControl::KB_INSERT;
    // systemToDavaKeyTranslator[125] = eInputControl::KB_FORWARD;
    // systemToDavaKeyTranslator[126] = eInputControl::KB_MEDIA_PLAY;
    // systemToDavaKeyTranslator[127] = eInputControl::KB_MEDIA_PAUSE;
    // systemToDavaKeyTranslator[128] = eInputControl::KB_MEDIA_CLOSE;
    // systemToDavaKeyTranslator[129] = eInputControl::KB_MEDIA_EJECT;
    // systemToDavaKeyTranslator[130] = eInputControl::KB_MEDIA_RECORD;
    systemToDavaKeyTranslator[131] = eInputControl::KB_F1;
    systemToDavaKeyTranslator[132] = eInputControl::KB_F2;
    systemToDavaKeyTranslator[133] = eInputControl::KB_F3;
    systemToDavaKeyTranslator[134] = eInputControl::KB_F4;
    systemToDavaKeyTranslator[135] = eInputControl::KB_F5;
    systemToDavaKeyTranslator[136] = eInputControl::KB_F6;
    systemToDavaKeyTranslator[137] = eInputControl::KB_F7;
    systemToDavaKeyTranslator[138] = eInputControl::KB_F8;
    systemToDavaKeyTranslator[139] = eInputControl::KB_F9;
    systemToDavaKeyTranslator[140] = eInputControl::KB_F10;
    systemToDavaKeyTranslator[141] = eInputControl::KB_F11;
    systemToDavaKeyTranslator[142] = eInputControl::KB_F12;
    systemToDavaKeyTranslator[143] = eInputControl::KB_NUMLOCK;
    systemToDavaKeyTranslator[144] = eInputControl::KB_NUMPAD0;
    systemToDavaKeyTranslator[145] = eInputControl::KB_NUMPAD1;
    systemToDavaKeyTranslator[146] = eInputControl::KB_NUMPAD2;
    systemToDavaKeyTranslator[147] = eInputControl::KB_NUMPAD3;
    systemToDavaKeyTranslator[148] = eInputControl::KB_NUMPAD4;
    systemToDavaKeyTranslator[149] = eInputControl::KB_NUMPAD5;
    systemToDavaKeyTranslator[150] = eInputControl::KB_NUMPAD6;
    systemToDavaKeyTranslator[151] = eInputControl::KB_NUMPAD7;
    systemToDavaKeyTranslator[152] = eInputControl::KB_NUMPAD8;
    systemToDavaKeyTranslator[153] = eInputControl::KB_NUMPAD9;
    systemToDavaKeyTranslator[154] = eInputControl::KB_DIVIDE;
    systemToDavaKeyTranslator[155] = eInputControl::KB_MULTIPLY;
    systemToDavaKeyTranslator[156] = eInputControl::KB_SUBTRACT;
    systemToDavaKeyTranslator[157] = eInputControl::KB_ADD;
    systemToDavaKeyTranslator[158] = eInputControl::KB_DECIMAL;
    // systemToDavaKeyTranslator[159] = eInputControl::KB_NUMPAD_COMMA;
    systemToDavaKeyTranslator[160] = eInputControl::KB_NUMPADENTER;
    systemToDavaKeyTranslator[161] = eInputControl::KB_EQUALS;
// systemToDavaKeyTranslator[162] = eInputControl::KB_NUMPAD_LEFT_PAREN;
// systemToDavaKeyTranslator[163] = eInputControl::KB_NUMPAD_RIGHT_PAREN;
// systemToDavaKeyTranslator[164] = eInputControl::KB_VOLUME_MUTE;
// systemToDavaKeyTranslator[165] = eInputControl::KB_INFO;
// systemToDavaKeyTranslator[166] = eInputControl::KB_CHANNEL_UP;
// systemToDavaKeyTranslator[167] = eInputControl::KB_CHANNEL_DOWN;
// systemToDavaKeyTranslator[168] = eInputControl::KB_ZOOM_IN;
// systemToDavaKeyTranslator[169] = eInputControl::KB_ZOOM_OUT;
// systemToDavaKeyTranslator[170] = eInputControl::KB_TV;
// systemToDavaKeyTranslator[171] = eInputControl::KB_WINDOW;
// systemToDavaKeyTranslator[172] = eInputControl::KB_GUIDE;
// systemToDavaKeyTranslator[173] = eInputControl::KB_DVR;
// systemToDavaKeyTranslator[174] = eInputControl::KB_BOOKMARK;
// systemToDavaKeyTranslator[175] = eInputControl::KB_CAPTIONS;
// systemToDavaKeyTranslator[176] = eInputControl::KB_SETTINGS;
// systemToDavaKeyTranslator[177] = eInputControl::KB_TV_POWER;
// systemToDavaKeyTranslator[178] = eInputControl::KB_TV_INPUT;
// systemToDavaKeyTranslator[179] = eInputControl::KB_STB_POWER;
// systemToDavaKeyTranslator[180] = eInputControl::KB_STB_INPUT;
// systemToDavaKeyTranslator[181] = eInputControl::KB_AVR_POWER;
// systemToDavaKeyTranslator[182] = eInputControl::KB_AVR_INPUT;
// systemToDavaKeyTranslator[183] = eInputControl::KB_PROG_RED;
// systemToDavaKeyTranslator[184] = eInputControl::KB_PROG_GREEN;
// systemToDavaKeyTranslator[185] = eInputControl::KB_PROG_YELLOW;
// systemToDavaKeyTranslator[186] = eInputControl::KB_PROG_BLUE;
// systemToDavaKeyTranslator[187] = eInputControl::KB_APP_SWITCH;
// systemToDavaKeyTranslator[188] = eInputControl::KB_BUTTON_1;
// systemToDavaKeyTranslator[189] = eInputControl::KB_BUTTON_2;
// systemToDavaKeyTranslator[190] = eInputControl::KB_BUTTON_3;
// systemToDavaKeyTranslator[191] = eInputControl::KB_BUTTON_4;
// systemToDavaKeyTranslator[192] = eInputControl::KB_BUTTON_5;
// systemToDavaKeyTranslator[193] = eInputControl::KB_BUTTON_6;
// systemToDavaKeyTranslator[194] = eInputControl::KB_BUTTON_7;
// systemToDavaKeyTranslator[195] = eInputControl::KB_BUTTON_8;
// systemToDavaKeyTranslator[196] = eInputControl::KB_BUTTON_9;
// systemToDavaKeyTranslator[197] = eInputControl::KB_BUTTON_10;
// systemToDavaKeyTranslator[198] = eInputControl::KB_BUTTON_11;
// systemToDavaKeyTranslator[199] = eInputControl::KB_BUTTON_12;
// systemToDavaKeyTranslator[200] = eInputControl::KB_BUTTON_13;
// systemToDavaKeyTranslator[201] = eInputControl::KB_BUTTON_14;
// systemToDavaKeyTranslator[202] = eInputControl::KB_BUTTON_15;
// systemToDavaKeyTranslator[203] = eInputControl::KB_BUTTON_16;
// systemToDavaKeyTranslator[204] = eInputControl::KB_LANGUAGE_SWITCH;
// systemToDavaKeyTranslator[205] = eInputControl::KB_MANNER_MODE;
// systemToDavaKeyTranslator[206] = eInputControl::KB_3D_MODE;
// systemToDavaKeyTranslator[207] = eInputControl::KB_CONTACTS;
// systemToDavaKeyTranslator[208] = eInputControl::KB_CALENDAR;
// systemToDavaKeyTranslator[209] = eInputControl::KB_MUSIC;
// systemToDavaKeyTranslator[210] = eInputControl::KB_CALCULATOR;
// systemToDavaKeyTranslator[211] = eInputControl::KB_ZENKAKU_HANKAKU;
// systemToDavaKeyTranslator[212] = eInputControl::KB_EISU;
// systemToDavaKeyTranslator[213] = eInputControl::KB_MUHENKAN;
// systemToDavaKeyTranslator[214] = eInputControl::KB_HENKAN;
// systemToDavaKeyTranslator[215] = eInputControl::KB_KATAKANA_HIRAGANA;
// systemToDavaKeyTranslator[216] = eInputControl::KB_YEN;
// systemToDavaKeyTranslator[217] = eInputControl::KB_RO;
// systemToDavaKeyTranslator[218] = eInputControl::KB_KANA;
// systemToDavaKeyTranslator[219] = eInputControl::KB_ASSIST;
// systemToDavaKeyTranslator[220] = eInputControl::KB_BRIGHTNESS_DOWN;
// systemToDavaKeyTranslator[221] = eInputControl::KB_BRIGHTNESS_UP;
// systemToDavaKeyTranslator[222] = eInputControl::KB_MEDIA_AUDIO_TRACK;
// systemToDavaKeyTranslator[223] = eInputControl::KB_SLEEP;
// systemToDavaKeyTranslator[224] = eInputControl::KB_WAKEUP;
// systemToDavaKeyTranslator[225] = eInputControl::KB_PAIRING;
// systemToDavaKeyTranslator[226] = eInputControl::KB_MEDIA_TOP_MENU;
// systemToDavaKeyTranslator[227] = eInputControl::KB_11;
// systemToDavaKeyTranslator[228] = eInputControl::KB_12;
// systemToDavaKeyTranslator[229] = eInputControl::KB_LAST_CHANNEL;
// systemToDavaKeyTranslator[230] = eInputControl::KB_TV_DATA_SERVICE;
// systemToDavaKeyTranslator[231] = eInputControl::KB_VOICE_ASSIST;
// systemToDavaKeyTranslator[232] = eInputControl::KB_TV_RADIO_SERVICE;
// systemToDavaKeyTranslator[233] = eInputControl::KB_TV_TELETEXT;
// systemToDavaKeyTranslator[234] = eInputControl::KB_TV_NUMBER_ENTRY;
// systemToDavaKeyTranslator[235] = eInputControl::KB_TV_TERRESTRIAL_ANALOG;
// systemToDavaKeyTranslator[236] = eInputControl::KB_TV_TERRESTRIAL_DIGITAL;
// systemToDavaKeyTranslator[237] = eInputControl::KB_TV_SATELLITE;
// systemToDavaKeyTranslator[238] = eInputControl::KB_TV_SATELLITE_BS;
// systemToDavaKeyTranslator[239] = eInputControl::KB_TV_SATELLITE_CS;
// systemToDavaKeyTranslator[240] = eInputControl::KB_TV_SATELLITE_SERVICE;
// systemToDavaKeyTranslator[241] = eInputControl::KB_TV_NETWORK;
// systemToDavaKeyTranslator[242] = eInputControl::KB_TV_ANTENNA_CABLE;
// systemToDavaKeyTranslator[243] = eInputControl::KB_TV_INPUT_HDMI_1;
// systemToDavaKeyTranslator[244] = eInputControl::KB_TV_INPUT_HDMI_2;
// systemToDavaKeyTranslator[245] = eInputControl::KB_TV_INPUT_HDMI_3;
// systemToDavaKeyTranslator[246] = eInputControl::KB_TV_INPUT_HDMI_4;
// systemToDavaKeyTranslator[247] = eInputControl::KB_TV_INPUT_COMPOSITE_1;
// systemToDavaKeyTranslator[248] = eInputControl::KB_TV_INPUT_COMPOSITE_2;
// systemToDavaKeyTranslator[249] = eInputControl::KB_TV_INPUT_COMPONENT_1;
// systemToDavaKeyTranslator[250] = eInputControl::KB_TV_INPUT_COMPONENT_2;
// systemToDavaKeyTranslator[251] = eInputControl::KB_TV_INPUT_VGA_1;
// systemToDavaKeyTranslator[252] = eInputControl::KB_TV_AUDIO_DESCRIPTION;
// systemToDavaKeyTranslator[253] = eInputControl::KB_TV_AUDIO_DESCRIPTION_MIX_UP;
// systemToDavaKeyTranslator[254] = eInputControl::KB_TV_AUDIO_DESCRIPTION_MIX_DOWN;
// systemToDavaKeyTranslator[255] = eInputControl::KB_TV_ZOOM_MODE;
// systemToDavaKeyTranslator[256] = eInputControl::KB_TV_CONTENTS_MENU;
// systemToDavaKeyTranslator[257] = eInputControl::KB_TV_MEDIA_CONTEXT_MENU;
// systemToDavaKeyTranslator[258] = eInputControl::KB_TV_TIMER_PROGRAMMING;
// systemToDavaKeyTranslator[259] = eInputControl::KB_HELP;
#endif
}

eInputControl SystemKeyToDavaKey(uint32 systemKeyCode)
{
    static bool translatorInitialized = false;
    if (!translatorInitialized)
    {
        InitSystemToDavaKeyTranslator();
        translatorInitialized = true;
    }

    if (systemKeyCode < MAX_TRANSLATOR_KEYS)
    {
        return systemToDavaKeyTranslator[systemKeyCode];
    }

    DVASSERT(false, "Wrong system key code");
    return eInputControl::NONE;
}
}