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
static Array<eKeyboardKey, MAX_TRANSLATOR_KEYS> systemToDavaKeyTranslator;

static void InitSystemToDavaKeyTranslator()
{
	static_assert(static_cast<size_t>(eKeyboardKey::TOTAL_KEYS_COUNT) < MAX_TRANSLATOR_KEYS, "Check translator array size");

    std::uninitialized_fill(std::begin(systemToDavaKeyTranslator), std::end(systemToDavaKeyTranslator), eKeyboardKey::UNKNOWN);

#if defined(__DAVAENGINE_WINDOWS__)
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx

    systemToDavaKeyTranslator[VK_LEFT] = eKeyboardKey::LEFT;
    systemToDavaKeyTranslator[VK_RIGHT] = eKeyboardKey::RIGHT;
    systemToDavaKeyTranslator[VK_UP] = eKeyboardKey::UP;
    systemToDavaKeyTranslator[VK_DOWN] = eKeyboardKey::DOWN;
    systemToDavaKeyTranslator[VK_DELETE] = eKeyboardKey::DELETE_;
    systemToDavaKeyTranslator[VK_ESCAPE] = eKeyboardKey::ESCAPE;
    systemToDavaKeyTranslator[VK_BACK] = eKeyboardKey::BACKSPACE;
    systemToDavaKeyTranslator[VK_RETURN] = eKeyboardKey::ENTER;

    systemToDavaKeyTranslator[256 + VK_LEFT] = eKeyboardKey::LEFT; // extended key
    systemToDavaKeyTranslator[256 + VK_RIGHT] = eKeyboardKey::RIGHT; // extended key
    systemToDavaKeyTranslator[256 + VK_UP] = eKeyboardKey::UP; // extended key
    systemToDavaKeyTranslator[256 + VK_DOWN] = eKeyboardKey::DOWN; // extended key
    systemToDavaKeyTranslator[256 + VK_DELETE] = eKeyboardKey::DELETE_; // extended key
    systemToDavaKeyTranslator[256 + VK_RETURN] = eKeyboardKey::NUMPADENTER; // extended key

    systemToDavaKeyTranslator[VK_CONTROL] = eKeyboardKey::LCTRL;
    systemToDavaKeyTranslator[VK_MENU] = eKeyboardKey::LALT;
    systemToDavaKeyTranslator[VK_SHIFT] = eKeyboardKey::LSHIFT;
    systemToDavaKeyTranslator[VK_APPS] = eKeyboardKey::APPS;

    systemToDavaKeyTranslator[256 + VK_CONTROL] = eKeyboardKey::RCTRL;
    systemToDavaKeyTranslator[256 + VK_MENU] = eKeyboardKey::RALT;
    systemToDavaKeyTranslator[256 + VK_SHIFT] = eKeyboardKey::RSHIFT;
    systemToDavaKeyTranslator[256 + VK_APPS] = eKeyboardKey::APPS; // win api mark this key as extended

    systemToDavaKeyTranslator[256 + VK_NUMLOCK] = eKeyboardKey::NUMLOCK;
    systemToDavaKeyTranslator[VK_CAPITAL] = eKeyboardKey::CAPSLOCK;
    systemToDavaKeyTranslator[VK_PAUSE] = eKeyboardKey::PAUSE;
    systemToDavaKeyTranslator[VK_SCROLL] = eKeyboardKey::SCROLLLOCK;
    systemToDavaKeyTranslator[256 + VK_SNAPSHOT] = eKeyboardKey::PRINTSCREEN;
    systemToDavaKeyTranslator[VK_SPACE] = eKeyboardKey::SPACE;
    systemToDavaKeyTranslator[VK_TAB] = eKeyboardKey::TAB;
    systemToDavaKeyTranslator[VK_ADD] = eKeyboardKey::ADD;
    systemToDavaKeyTranslator[VK_SUBTRACT] = eKeyboardKey::SUBTRACT;

    systemToDavaKeyTranslator[VK_HOME] = eKeyboardKey::HOME;
    systemToDavaKeyTranslator[VK_END] = eKeyboardKey::END;
    systemToDavaKeyTranslator[VK_PRIOR] = eKeyboardKey::PGUP;
    systemToDavaKeyTranslator[VK_NEXT] = eKeyboardKey::PGDN;
    systemToDavaKeyTranslator[VK_INSERT] = eKeyboardKey::INSERT;
    systemToDavaKeyTranslator[256 + VK_HOME] = eKeyboardKey::HOME; // extended key
    systemToDavaKeyTranslator[256 + VK_END] = eKeyboardKey::END; // extended key
    systemToDavaKeyTranslator[256 + VK_PRIOR] = eKeyboardKey::PGUP; // extended key
    systemToDavaKeyTranslator[256 + VK_NEXT] = eKeyboardKey::PGDN; // extended key
    systemToDavaKeyTranslator[256 + VK_INSERT] = eKeyboardKey::INSERT; // extended key

    systemToDavaKeyTranslator[VK_OEM_PLUS] = eKeyboardKey::EQUALS;
    systemToDavaKeyTranslator[VK_OEM_MINUS] = eKeyboardKey::MINUS;
    systemToDavaKeyTranslator[VK_OEM_PERIOD] = eKeyboardKey::PERIOD;
    systemToDavaKeyTranslator[VK_OEM_COMMA] = eKeyboardKey::COMMA;
    systemToDavaKeyTranslator[VK_OEM_1] = eKeyboardKey::SEMICOLON;
    systemToDavaKeyTranslator[VK_OEM_2] = eKeyboardKey::SLASH;
    systemToDavaKeyTranslator[VK_OEM_3] = eKeyboardKey::GRAVE;
    systemToDavaKeyTranslator[VK_OEM_4] = eKeyboardKey::LBRACKET;
    systemToDavaKeyTranslator[VK_OEM_5] = eKeyboardKey::BACKSLASH;
    systemToDavaKeyTranslator[VK_OEM_6] = eKeyboardKey::RBRACKET;
    systemToDavaKeyTranslator[VK_OEM_7] = eKeyboardKey::APOSTROPHE;

    systemToDavaKeyTranslator[VK_OEM_102] = eKeyboardKey::NONUSBACKSLASH;

    const unsigned numFuncKeys = static_cast<unsigned>(eKeyboardKey::F12) - static_cast<unsigned>(eKeyboardKey::F1);
    for (unsigned i = 0; i <= numFuncKeys; i++)
    {
        unsigned keyValue = static_cast<unsigned>(eKeyboardKey::F1) + i;
        systemToDavaKeyTranslator[VK_F1 + i] = static_cast<eKeyboardKey>(keyValue);
    }

    // alpha keys
    for (unsigned i = 0; i < 26; ++i)
    {
        unsigned keyValue = static_cast<unsigned>(eKeyboardKey::KEY_A) + i;
        systemToDavaKeyTranslator[0x41 + i] = static_cast<eKeyboardKey>(keyValue);
    }

    // numeric keys & keys at num pad
    for (unsigned i = 0; i < 10; ++i)
    {
        unsigned keyNum = static_cast<unsigned>(eKeyboardKey::KEY_0) + i;
        unsigned keyNumpad = static_cast<unsigned>(eKeyboardKey::NUMPAD0) + i;
        systemToDavaKeyTranslator[0x30 + i] = static_cast<eKeyboardKey>(keyNum);
        systemToDavaKeyTranslator[0x60 + i] = static_cast<eKeyboardKey>(keyNumpad);
    }
    systemToDavaKeyTranslator[VK_MULTIPLY] = eKeyboardKey::MULTIPLY;
    systemToDavaKeyTranslator[256 + VK_DIVIDE] = eKeyboardKey::DIVIDE; // extended key
    systemToDavaKeyTranslator[VK_DECIMAL] = eKeyboardKey::DECIMAL;
#endif

#if defined(__DAVAENGINE_MACOS__)
    systemToDavaKeyTranslator[0x7B] = eKeyboardKey::LEFT;
    systemToDavaKeyTranslator[0x7C] = eKeyboardKey::RIGHT;
    systemToDavaKeyTranslator[0x7E] = eKeyboardKey::UP;
    systemToDavaKeyTranslator[0x7D] = eKeyboardKey::DOWN;
    systemToDavaKeyTranslator[0x75] = eKeyboardKey::DELETE;
    systemToDavaKeyTranslator[0x35] = eKeyboardKey::ESCAPE;
    systemToDavaKeyTranslator[0x33] = eKeyboardKey::BACKSPACE;
    systemToDavaKeyTranslator[0x24] = eKeyboardKey::ENTER;
    systemToDavaKeyTranslator[0x30] = eKeyboardKey::TAB;

    systemToDavaKeyTranslator[59] = eKeyboardKey::LCTRL;
    systemToDavaKeyTranslator[58] = eKeyboardKey::LALT;
    systemToDavaKeyTranslator[56] = eKeyboardKey::LSHIFT;
    systemToDavaKeyTranslator[62] = eKeyboardKey::RCTRL;
    systemToDavaKeyTranslator[61] = eKeyboardKey::RALT;
    systemToDavaKeyTranslator[60] = eKeyboardKey::RSHIFT;

    systemToDavaKeyTranslator[57] = eKeyboardKey::CAPSLOCK;
    systemToDavaKeyTranslator[54] = eKeyboardKey::RCMD;
    systemToDavaKeyTranslator[55] = eKeyboardKey::LCMD; // LGUI in SDL
    systemToDavaKeyTranslator[0x31] = eKeyboardKey::SPACE;

    // from SDL2 scancodes_darwin.h
    systemToDavaKeyTranslator[10] = eKeyboardKey::NONUSBACKSLASH;
    systemToDavaKeyTranslator[24] = eKeyboardKey::EQUALS;
    systemToDavaKeyTranslator[27] = eKeyboardKey::MINUS;
    systemToDavaKeyTranslator[47] = eKeyboardKey::PERIOD;
    systemToDavaKeyTranslator[43] = eKeyboardKey::COMMA;
    systemToDavaKeyTranslator[41] = eKeyboardKey::SEMICOLON;
    systemToDavaKeyTranslator[44] = eKeyboardKey::SLASH;
    systemToDavaKeyTranslator[50] = eKeyboardKey::GRAVE;
    systemToDavaKeyTranslator[33] = eKeyboardKey::LBRACKET;
    systemToDavaKeyTranslator[42] = eKeyboardKey::BACKSLASH;
    systemToDavaKeyTranslator[30] = eKeyboardKey::RBRACKET;
    systemToDavaKeyTranslator[39] = eKeyboardKey::APOSTROPHE;
    systemToDavaKeyTranslator[114] = eKeyboardKey::INSERT;
    systemToDavaKeyTranslator[115] = eKeyboardKey::HOME;
    systemToDavaKeyTranslator[116] = eKeyboardKey::PGUP;
    systemToDavaKeyTranslator[119] = eKeyboardKey::END;
    systemToDavaKeyTranslator[121] = eKeyboardKey::PGDN;
    systemToDavaKeyTranslator[69] = eKeyboardKey::ADD;
    systemToDavaKeyTranslator[78] = eKeyboardKey::MINUS;
    systemToDavaKeyTranslator[67] = eKeyboardKey::MULTIPLY;
    systemToDavaKeyTranslator[75] = eKeyboardKey::DIVIDE;
    systemToDavaKeyTranslator[81] = eKeyboardKey::EQUALS;
    systemToDavaKeyTranslator[65] = eKeyboardKey::PERIOD;

    systemToDavaKeyTranslator[0x00] = eKeyboardKey::KEY_A;
    systemToDavaKeyTranslator[0x0B] = eKeyboardKey::KEY_B;
    systemToDavaKeyTranslator[0x08] = eKeyboardKey::KEY_C;
    systemToDavaKeyTranslator[0x02] = eKeyboardKey::KEY_D;
    systemToDavaKeyTranslator[0x0E] = eKeyboardKey::KEY_E;
    systemToDavaKeyTranslator[0x03] = eKeyboardKey::KEY_F;
    systemToDavaKeyTranslator[0x05] = eKeyboardKey::KEY_G;
    systemToDavaKeyTranslator[0x04] = eKeyboardKey::KEY_H;
    systemToDavaKeyTranslator[0x22] = eKeyboardKey::KEY_I;
    systemToDavaKeyTranslator[0x26] = eKeyboardKey::KEY_J;
    systemToDavaKeyTranslator[0x28] = eKeyboardKey::KEY_K;
    systemToDavaKeyTranslator[0x25] = eKeyboardKey::KEY_L;
    systemToDavaKeyTranslator[0x2D] = eKeyboardKey::KEY_N;
    systemToDavaKeyTranslator[0x2E] = eKeyboardKey::KEY_M;
    systemToDavaKeyTranslator[0x1F] = eKeyboardKey::KEY_O;
    systemToDavaKeyTranslator[0x23] = eKeyboardKey::KEY_P;
    systemToDavaKeyTranslator[0x0C] = eKeyboardKey::KEY_Q;
    systemToDavaKeyTranslator[0x0F] = eKeyboardKey::KEY_R;
    systemToDavaKeyTranslator[0x01] = eKeyboardKey::KEY_S;
    systemToDavaKeyTranslator[0x11] = eKeyboardKey::KEY_T;
    systemToDavaKeyTranslator[0x20] = eKeyboardKey::KEY_U;
    systemToDavaKeyTranslator[0x09] = eKeyboardKey::KEY_V;
    systemToDavaKeyTranslator[0x0D] = eKeyboardKey::KEY_W;
    systemToDavaKeyTranslator[0x07] = eKeyboardKey::KEY_X;
    systemToDavaKeyTranslator[0x10] = eKeyboardKey::KEY_Y;
    systemToDavaKeyTranslator[0x06] = eKeyboardKey::KEY_Z;

    systemToDavaKeyTranslator[0x1D] = eKeyboardKey::KEY_0;
    systemToDavaKeyTranslator[0x12] = eKeyboardKey::KEY_1;
    systemToDavaKeyTranslator[0x13] = eKeyboardKey::KEY_2;
    systemToDavaKeyTranslator[0x14] = eKeyboardKey::KEY_3;
    systemToDavaKeyTranslator[0x15] = eKeyboardKey::KEY_4;
    systemToDavaKeyTranslator[0x17] = eKeyboardKey::KEY_5;
    systemToDavaKeyTranslator[0x16] = eKeyboardKey::KEY_6;
    systemToDavaKeyTranslator[0x1A] = eKeyboardKey::KEY_7;
    systemToDavaKeyTranslator[0x1C] = eKeyboardKey::KEY_8;
    systemToDavaKeyTranslator[0x19] = eKeyboardKey::KEY_9;
    systemToDavaKeyTranslator[0x1B] = eKeyboardKey::MINUS;
    systemToDavaKeyTranslator[0x18] = eKeyboardKey::EQUALS;

    systemToDavaKeyTranslator[0x7A] = eKeyboardKey::F1;
    systemToDavaKeyTranslator[0x78] = eKeyboardKey::F2;
    systemToDavaKeyTranslator[0x76] = eKeyboardKey::F4;
    systemToDavaKeyTranslator[0x60] = eKeyboardKey::F5;
    systemToDavaKeyTranslator[0x61] = eKeyboardKey::F6;
    systemToDavaKeyTranslator[0x62] = eKeyboardKey::F7;
    systemToDavaKeyTranslator[0x63] = eKeyboardKey::F3;
    systemToDavaKeyTranslator[0x64] = eKeyboardKey::F8;
    systemToDavaKeyTranslator[0x65] = eKeyboardKey::F9;
    systemToDavaKeyTranslator[0x6D] = eKeyboardKey::F10;
    systemToDavaKeyTranslator[0x67] = eKeyboardKey::F11;
    systemToDavaKeyTranslator[0x6F] = eKeyboardKey::F12;

    systemToDavaKeyTranslator[113] = eKeyboardKey::F15;
    systemToDavaKeyTranslator[0x6A] = eKeyboardKey::F16;
    systemToDavaKeyTranslator[0x40] = eKeyboardKey::F17;
    systemToDavaKeyTranslator[0x4F] = eKeyboardKey::F18;
    systemToDavaKeyTranslator[0x50] = eKeyboardKey::F19;

    // numeric keys at numpad
    for (unsigned i = 0; i < 8; ++i)
    {
        systemToDavaKeyTranslator[0x52 + i] = static_cast<eKeyboardKey>(static_cast<unsigned>(eKeyboardKey::NUMPAD0) + i);
    }
    systemToDavaKeyTranslator[91] = eKeyboardKey::NUMPAD8;
    systemToDavaKeyTranslator[92] = eKeyboardKey::NUMPAD9;

    systemToDavaKeyTranslator[71] = eKeyboardKey::NUMLOCK;
    systemToDavaKeyTranslator[76] = eKeyboardKey::NUMPADENTER;
    systemToDavaKeyTranslator[65] = eKeyboardKey::DECIMAL;
    systemToDavaKeyTranslator[110] = eKeyboardKey::APPS;
    systemToDavaKeyTranslator[107] = eKeyboardKey::SCROLLLOCK;
    systemToDavaKeyTranslator[105] = eKeyboardKey::PRINTSCREEN;
#endif

#if defined(__DAVAENGINE_ANDROID__)
    systemToDavaKeyTranslator[0] = eKeyboardKey::UNKNOWN;
    systemToDavaKeyTranslator[1] = eKeyboardKey::LEFT;
    systemToDavaKeyTranslator[2] = eKeyboardKey::RIGHT;
    systemToDavaKeyTranslator[3] = eKeyboardKey::HOME;
    systemToDavaKeyTranslator[4] = eKeyboardKey::BACK;
    // systemToDavaKeyTranslator[5] = eKeyboardKey::CALL;
    // systemToDavaKeyTranslator[6] = eKeyboardKey::ENDCALL;
    systemToDavaKeyTranslator[7] = eKeyboardKey::KEY_0;
    systemToDavaKeyTranslator[8] = eKeyboardKey::KEY_1;
    systemToDavaKeyTranslator[9] = eKeyboardKey::KEY_2;
    systemToDavaKeyTranslator[10] = eKeyboardKey::KEY_3;
    systemToDavaKeyTranslator[11] = eKeyboardKey::KEY_4;
    systemToDavaKeyTranslator[12] = eKeyboardKey::KEY_5;
    systemToDavaKeyTranslator[13] = eKeyboardKey::KEY_6;
    systemToDavaKeyTranslator[14] = eKeyboardKey::KEY_7;
    systemToDavaKeyTranslator[15] = eKeyboardKey::KEY_8;
    systemToDavaKeyTranslator[16] = eKeyboardKey::KEY_9;
    // systemToDavaKeyTranslator[17] = eKeyboardKey::STAR;
    // systemToDavaKeyTranslator[18] = eKeyboardKey::POUND;
    // systemToDavaKeyTranslator[19] = eKeyboardKey::DPAD_UP;
    // systemToDavaKeyTranslator[20] = eKeyboardKey::DPAD_DOWN;
    // systemToDavaKeyTranslator[21] = eKeyboardKey::DPAD_LEFT;
    // systemToDavaKeyTranslator[22] = eKeyboardKey::DPAD_RIGHT;
    // systemToDavaKeyTranslator[23] = eKeyboardKey::DPAD_CENTER;
    // systemToDavaKeyTranslator[24] = eKeyboardKey::VOLUME_UP;
    // systemToDavaKeyTranslator[25] = eKeyboardKey::VOLUME_DOWN;
    // systemToDavaKeyTranslator[26] = eKeyboardKey::POWER;
    // systemToDavaKeyTranslator[27] = eKeyboardKey::CAMERA;
    // systemToDavaKeyTranslator[28] = eKeyboardKey::CLEAR;
    systemToDavaKeyTranslator[29] = eKeyboardKey::KEY_A;
    systemToDavaKeyTranslator[30] = eKeyboardKey::KEY_B;
    systemToDavaKeyTranslator[31] = eKeyboardKey::KEY_C;
    systemToDavaKeyTranslator[32] = eKeyboardKey::KEY_D;
    systemToDavaKeyTranslator[33] = eKeyboardKey::KEY_E;
    systemToDavaKeyTranslator[34] = eKeyboardKey::KEY_F;
    systemToDavaKeyTranslator[35] = eKeyboardKey::KEY_G;
    systemToDavaKeyTranslator[36] = eKeyboardKey::KEY_H;
    systemToDavaKeyTranslator[37] = eKeyboardKey::KEY_I;
    systemToDavaKeyTranslator[38] = eKeyboardKey::KEY_J;
    systemToDavaKeyTranslator[39] = eKeyboardKey::KEY_K;
    systemToDavaKeyTranslator[40] = eKeyboardKey::KEY_L;
    systemToDavaKeyTranslator[41] = eKeyboardKey::KEY_M;
    systemToDavaKeyTranslator[42] = eKeyboardKey::KEY_N;
    systemToDavaKeyTranslator[43] = eKeyboardKey::KEY_O;
    systemToDavaKeyTranslator[44] = eKeyboardKey::KEY_P;
    systemToDavaKeyTranslator[45] = eKeyboardKey::KEY_Q;
    systemToDavaKeyTranslator[46] = eKeyboardKey::KEY_R;
    systemToDavaKeyTranslator[47] = eKeyboardKey::KEY_S;
    systemToDavaKeyTranslator[48] = eKeyboardKey::KEY_T;
    systemToDavaKeyTranslator[49] = eKeyboardKey::KEY_U;
    systemToDavaKeyTranslator[50] = eKeyboardKey::KEY_V;
    systemToDavaKeyTranslator[51] = eKeyboardKey::KEY_W;
    systemToDavaKeyTranslator[52] = eKeyboardKey::KEY_X;
    systemToDavaKeyTranslator[53] = eKeyboardKey::KEY_Y;
    systemToDavaKeyTranslator[54] = eKeyboardKey::KEY_Z;
    systemToDavaKeyTranslator[55] = eKeyboardKey::COMMA;
    systemToDavaKeyTranslator[56] = eKeyboardKey::PERIOD;
    systemToDavaKeyTranslator[57] = eKeyboardKey::LALT;
    systemToDavaKeyTranslator[58] = eKeyboardKey::RALT;
    systemToDavaKeyTranslator[59] = eKeyboardKey::LSHIFT;
    systemToDavaKeyTranslator[60] = eKeyboardKey::RSHIFT;
    systemToDavaKeyTranslator[61] = eKeyboardKey::TAB;
    systemToDavaKeyTranslator[62] = eKeyboardKey::SPACE;
    // systemToDavaKeyTranslator[63] = eKeyboardKey::SYM;
    // systemToDavaKeyTranslator[64] = eKeyboardKey::EXPLORER;
    // systemToDavaKeyTranslator[65] = eKeyboardKey::ENVELOPE;
    systemToDavaKeyTranslator[66] = eKeyboardKey::ENTER;
    systemToDavaKeyTranslator[67] = eKeyboardKey::DELETE;
    systemToDavaKeyTranslator[68] = eKeyboardKey::GRAVE;
    systemToDavaKeyTranslator[69] = eKeyboardKey::MINUS;
    systemToDavaKeyTranslator[70] = eKeyboardKey::EQUALS;
    systemToDavaKeyTranslator[71] = eKeyboardKey::LBRACKET;
    systemToDavaKeyTranslator[72] = eKeyboardKey::RBRACKET;
    systemToDavaKeyTranslator[73] = eKeyboardKey::BACKSLASH;
    systemToDavaKeyTranslator[74] = eKeyboardKey::SEMICOLON;
    systemToDavaKeyTranslator[75] = eKeyboardKey::APOSTROPHE;
    systemToDavaKeyTranslator[76] = eKeyboardKey::SLASH;
    // systemToDavaKeyTranslator[77] = eKeyboardKey::AT;
    // systemToDavaKeyTranslator[78] = eKeyboardKey::NUM;
    // systemToDavaKeyTranslator[79] = eKeyboardKey::HEADSETHOOK;
    // systemToDavaKeyTranslator[80] = eKeyboardKey::FOCUS;
    // systemToDavaKeyTranslator[81] = eKeyboardKey::PLUS;
    systemToDavaKeyTranslator[82] = eKeyboardKey::MENU;
    // systemToDavaKeyTranslator[83] = eKeyboardKey::NOTIFICATION;
    // systemToDavaKeyTranslator[84] = eKeyboardKey::SEARCH;
    // systemToDavaKeyTranslator[85] = eKeyboardKey::MEDIA_PLAY_PAUSE;
    // systemToDavaKeyTranslator[86] = eKeyboardKey::MEDIA_STOP;
    // systemToDavaKeyTranslator[87] = eKeyboardKey::MEDIA_NEXT;
    // systemToDavaKeyTranslator[88] = eKeyboardKey::MEDIA_PREVIOUS;
    // systemToDavaKeyTranslator[89] = eKeyboardKey::MEDIA_REWIND;
    // systemToDavaKeyTranslator[90] = eKeyboardKey::MEDIA_FAST_FORWARD;
    // systemToDavaKeyTranslator[91] = eKeyboardKey::MUTE;
    systemToDavaKeyTranslator[92] = eKeyboardKey::PGUP;
    systemToDavaKeyTranslator[93] = eKeyboardKey::PGDN;
    // systemToDavaKeyTranslator[94] = eKeyboardKey::PICTSYMBOLS;
    // systemToDavaKeyTranslator[95] = eKeyboardKey::SWITCH_CHARSET;
    // systemToDavaKeyTranslator[96] = eKeyboardKey::BUTTON_A;
    // systemToDavaKeyTranslator[97] = eKeyboardKey::BUTTON_B;
    // systemToDavaKeyTranslator[98] = eKeyboardKey::BUTTON_C;
    // systemToDavaKeyTranslator[99] = eKeyboardKey::BUTTON_X;
    // systemToDavaKeyTranslator[100] = eKeyboardKey::BUTTON_Y;
    // systemToDavaKeyTranslator[101] = eKeyboardKey::BUTTON_Z;
    // systemToDavaKeyTranslator[102] = eKeyboardKey::BUTTON_L1;
    // systemToDavaKeyTranslator[103] = eKeyboardKey::BUTTON_R1;
    // systemToDavaKeyTranslator[104] = eKeyboardKey::BUTTON_L2;
    // systemToDavaKeyTranslator[105] = eKeyboardKey::BUTTON_R2;
    // systemToDavaKeyTranslator[106] = eKeyboardKey::BUTTON_THUMBL;
    // systemToDavaKeyTranslator[107] = eKeyboardKey::BUTTON_THUMBR;
    // systemToDavaKeyTranslator[108] = eKeyboardKey::BUTTON_START;
    // systemToDavaKeyTranslator[109] = eKeyboardKey::BUTTON_SELECT;
    // systemToDavaKeyTranslator[110] = eKeyboardKey::BUTTON_MODE;
    systemToDavaKeyTranslator[111] = eKeyboardKey::ESCAPE;
    // systemToDavaKeyTranslator[112] = eKeyboardKey::FORWARD_DEL;
    systemToDavaKeyTranslator[113] = eKeyboardKey::LCTRL;
    systemToDavaKeyTranslator[114] = eKeyboardKey::RCTRL;
    systemToDavaKeyTranslator[115] = eKeyboardKey::CAPSLOCK;
    systemToDavaKeyTranslator[116] = eKeyboardKey::SCROLLLOCK;
    systemToDavaKeyTranslator[117] = eKeyboardKey::LCMD;
    systemToDavaKeyTranslator[118] = eKeyboardKey::RCMD;
    // systemToDavaKeyTranslator[119] = eKeyboardKey::FUNCTION;
    // systemToDavaKeyTranslator[120] = eKeyboardKey::SYSRQ;
    systemToDavaKeyTranslator[121] = eKeyboardKey::PAUSE;
    systemToDavaKeyTranslator[122] = eKeyboardKey::HOME;
    systemToDavaKeyTranslator[123] = eKeyboardKey::END;
    systemToDavaKeyTranslator[124] = eKeyboardKey::INSERT;
    // systemToDavaKeyTranslator[125] = eKeyboardKey::FORWARD;
    // systemToDavaKeyTranslator[126] = eKeyboardKey::MEDIA_PLAY;
    // systemToDavaKeyTranslator[127] = eKeyboardKey::MEDIA_PAUSE;
    // systemToDavaKeyTranslator[128] = eKeyboardKey::MEDIA_CLOSE;
    // systemToDavaKeyTranslator[129] = eKeyboardKey::MEDIA_EJECT;
    // systemToDavaKeyTranslator[130] = eKeyboardKey::MEDIA_RECORD;
    systemToDavaKeyTranslator[131] = eKeyboardKey::F1;
    systemToDavaKeyTranslator[132] = eKeyboardKey::F2;
    systemToDavaKeyTranslator[133] = eKeyboardKey::F3;
    systemToDavaKeyTranslator[134] = eKeyboardKey::F4;
    systemToDavaKeyTranslator[135] = eKeyboardKey::F5;
    systemToDavaKeyTranslator[136] = eKeyboardKey::F6;
    systemToDavaKeyTranslator[137] = eKeyboardKey::F7;
    systemToDavaKeyTranslator[138] = eKeyboardKey::F8;
    systemToDavaKeyTranslator[139] = eKeyboardKey::F9;
    systemToDavaKeyTranslator[140] = eKeyboardKey::F10;
    systemToDavaKeyTranslator[141] = eKeyboardKey::F11;
    systemToDavaKeyTranslator[142] = eKeyboardKey::F12;
    systemToDavaKeyTranslator[143] = eKeyboardKey::NUMLOCK;
    systemToDavaKeyTranslator[144] = eKeyboardKey::NUMPAD0;
    systemToDavaKeyTranslator[145] = eKeyboardKey::NUMPAD1;
    systemToDavaKeyTranslator[146] = eKeyboardKey::NUMPAD2;
    systemToDavaKeyTranslator[147] = eKeyboardKey::NUMPAD3;
    systemToDavaKeyTranslator[148] = eKeyboardKey::NUMPAD4;
    systemToDavaKeyTranslator[149] = eKeyboardKey::NUMPAD5;
    systemToDavaKeyTranslator[150] = eKeyboardKey::NUMPAD6;
    systemToDavaKeyTranslator[151] = eKeyboardKey::NUMPAD7;
    systemToDavaKeyTranslator[152] = eKeyboardKey::NUMPAD8;
    systemToDavaKeyTranslator[153] = eKeyboardKey::NUMPAD9;
    systemToDavaKeyTranslator[154] = eKeyboardKey::DIVIDE;
    systemToDavaKeyTranslator[155] = eKeyboardKey::MULTIPLY;
    systemToDavaKeyTranslator[156] = eKeyboardKey::SUBTRACT;
    systemToDavaKeyTranslator[157] = eKeyboardKey::ADD;
    systemToDavaKeyTranslator[158] = eKeyboardKey::DECIMAL;
    // systemToDavaKeyTranslator[159] = eKeyboardKey::NUMPAD_COMMA;
    systemToDavaKeyTranslator[160] = eKeyboardKey::NUMPADENTER;
    systemToDavaKeyTranslator[161] = eKeyboardKey::EQUALS;
    // systemToDavaKeyTranslator[162] = eKeyboardKey::NUMPAD_LEFT_PAREN;
    // systemToDavaKeyTranslator[163] = eKeyboardKey::NUMPAD_RIGHT_PAREN;
    // systemToDavaKeyTranslator[164] = eKeyboardKey::VOLUME_MUTE;
    // systemToDavaKeyTranslator[165] = eKeyboardKey::INFO;
    // systemToDavaKeyTranslator[166] = eKeyboardKey::CHANNEL_UP;
    // systemToDavaKeyTranslator[167] = eKeyboardKey::CHANNEL_DOWN;
    // systemToDavaKeyTranslator[168] = eKeyboardKey::ZOOM_IN;
    // systemToDavaKeyTranslator[169] = eKeyboardKey::ZOOM_OUT;
    // systemToDavaKeyTranslator[170] = eKeyboardKey::TV;
    // systemToDavaKeyTranslator[171] = eKeyboardKey::WINDOW;
    // systemToDavaKeyTranslator[172] = eKeyboardKey::GUIDE;
    // systemToDavaKeyTranslator[173] = eKeyboardKey::DVR;
    // systemToDavaKeyTranslator[174] = eKeyboardKey::BOOKMARK;
    // systemToDavaKeyTranslator[175] = eKeyboardKey::CAPTIONS;
    // systemToDavaKeyTranslator[176] = eKeyboardKey::SETTINGS;
    // systemToDavaKeyTranslator[177] = eKeyboardKey::TV_POWER;
    // systemToDavaKeyTranslator[178] = eKeyboardKey::TV_INPUT;
    // systemToDavaKeyTranslator[179] = eKeyboardKey::STB_POWER;
    // systemToDavaKeyTranslator[180] = eKeyboardKey::STB_INPUT;
    // systemToDavaKeyTranslator[181] = eKeyboardKey::AVR_POWER;
    // systemToDavaKeyTranslator[182] = eKeyboardKey::AVR_INPUT;
    // systemToDavaKeyTranslator[183] = eKeyboardKey::PROG_RED;
    // systemToDavaKeyTranslator[184] = eKeyboardKey::PROG_GREEN;
    // systemToDavaKeyTranslator[185] = eKeyboardKey::PROG_YELLOW;
    // systemToDavaKeyTranslator[186] = eKeyboardKey::PROG_BLUE;
    // systemToDavaKeyTranslator[187] = eKeyboardKey::APP_SWITCH;
    // systemToDavaKeyTranslator[188] = eKeyboardKey::BUTTON_1;
    // systemToDavaKeyTranslator[189] = eKeyboardKey::BUTTON_2;
    // systemToDavaKeyTranslator[190] = eKeyboardKey::BUTTON_3;
    // systemToDavaKeyTranslator[191] = eKeyboardKey::BUTTON_4;
    // systemToDavaKeyTranslator[192] = eKeyboardKey::BUTTON_5;
    // systemToDavaKeyTranslator[193] = eKeyboardKey::BUTTON_6;
    // systemToDavaKeyTranslator[194] = eKeyboardKey::BUTTON_7;
    // systemToDavaKeyTranslator[195] = eKeyboardKey::BUTTON_8;
    // systemToDavaKeyTranslator[196] = eKeyboardKey::BUTTON_9;
    // systemToDavaKeyTranslator[197] = eKeyboardKey::BUTTON_10;
    // systemToDavaKeyTranslator[198] = eKeyboardKey::BUTTON_11;
    // systemToDavaKeyTranslator[199] = eKeyboardKey::BUTTON_12;
    // systemToDavaKeyTranslator[200] = eKeyboardKey::BUTTON_13;
    // systemToDavaKeyTranslator[201] = eKeyboardKey::BUTTON_14;
    // systemToDavaKeyTranslator[202] = eKeyboardKey::BUTTON_15;
    // systemToDavaKeyTranslator[203] = eKeyboardKey::BUTTON_16;
    // systemToDavaKeyTranslator[204] = eKeyboardKey::LANGUAGE_SWITCH;
    // systemToDavaKeyTranslator[205] = eKeyboardKey::MANNER_MODE;
    // systemToDavaKeyTranslator[206] = eKeyboardKey::3D_MODE;
    // systemToDavaKeyTranslator[207] = eKeyboardKey::CONTACTS;
    // systemToDavaKeyTranslator[208] = eKeyboardKey::CALENDAR;
    // systemToDavaKeyTranslator[209] = eKeyboardKey::MUSIC;
    // systemToDavaKeyTranslator[210] = eKeyboardKey::CALCULATOR;
    // systemToDavaKeyTranslator[211] = eKeyboardKey::ZENKAKU_HANKAKU;
    // systemToDavaKeyTranslator[212] = eKeyboardKey::EISU;
    // systemToDavaKeyTranslator[213] = eKeyboardKey::MUHENKAN;
    // systemToDavaKeyTranslator[214] = eKeyboardKey::HENKAN;
    // systemToDavaKeyTranslator[215] = eKeyboardKey::KATAKANA_HIRAGANA;
    // systemToDavaKeyTranslator[216] = eKeyboardKey::YEN;
    // systemToDavaKeyTranslator[217] = eKeyboardKey::RO;
    // systemToDavaKeyTranslator[218] = eKeyboardKey::KANA;
    // systemToDavaKeyTranslator[219] = eKeyboardKey::ASSIST;
    // systemToDavaKeyTranslator[220] = eKeyboardKey::BRIGHTNESS_DOWN;
    // systemToDavaKeyTranslator[221] = eKeyboardKey::BRIGHTNESS_UP;
    // systemToDavaKeyTranslator[222] = eKeyboardKey::MEDIA_AUDIO_TRACK;
    // systemToDavaKeyTranslator[223] = eKeyboardKey::SLEEP;
    // systemToDavaKeyTranslator[224] = eKeyboardKey::WAKEUP;
    // systemToDavaKeyTranslator[225] = eKeyboardKey::PAIRING;
    // systemToDavaKeyTranslator[226] = eKeyboardKey::MEDIA_TOP_MENU;
    // systemToDavaKeyTranslator[227] = eKeyboardKey::11;
    // systemToDavaKeyTranslator[228] = eKeyboardKey::12;
    // systemToDavaKeyTranslator[229] = eKeyboardKey::LAST_CHANNEL;
    // systemToDavaKeyTranslator[230] = eKeyboardKey::TV_DATA_SERVICE;
    // systemToDavaKeyTranslator[231] = eKeyboardKey::VOICE_ASSIST;
    // systemToDavaKeyTranslator[232] = eKeyboardKey::TV_RADIO_SERVICE;
    // systemToDavaKeyTranslator[233] = eKeyboardKey::TV_TELETEXT;
    // systemToDavaKeyTranslator[234] = eKeyboardKey::TV_NUMBER_ENTRY;
    // systemToDavaKeyTranslator[235] = eKeyboardKey::TV_TERRESTRIAL_ANALOG;
    // systemToDavaKeyTranslator[236] = eKeyboardKey::TV_TERRESTRIAL_DIGITAL;
    // systemToDavaKeyTranslator[237] = eKeyboardKey::TV_SATELLITE;
    // systemToDavaKeyTranslator[238] = eKeyboardKey::TV_SATELLITE_BS;
    // systemToDavaKeyTranslator[239] = eKeyboardKey::TV_SATELLITE_CS;
    // systemToDavaKeyTranslator[240] = eKeyboardKey::TV_SATELLITE_SERVICE;
    // systemToDavaKeyTranslator[241] = eKeyboardKey::TV_NETWORK;
    // systemToDavaKeyTranslator[242] = eKeyboardKey::TV_ANTENNA_CABLE;
    // systemToDavaKeyTranslator[243] = eKeyboardKey::TV_INPUT_HDMI_1;
    // systemToDavaKeyTranslator[244] = eKeyboardKey::TV_INPUT_HDMI_2;
    // systemToDavaKeyTranslator[245] = eKeyboardKey::TV_INPUT_HDMI_3;
    // systemToDavaKeyTranslator[246] = eKeyboardKey::TV_INPUT_HDMI_4;
    // systemToDavaKeyTranslator[247] = eKeyboardKey::TV_INPUT_COMPOSITE_1;
    // systemToDavaKeyTranslator[248] = eKeyboardKey::TV_INPUT_COMPOSITE_2;
    // systemToDavaKeyTranslator[249] = eKeyboardKey::TV_INPUT_COMPONENT_1;
    // systemToDavaKeyTranslator[250] = eKeyboardKey::TV_INPUT_COMPONENT_2;
    // systemToDavaKeyTranslator[251] = eKeyboardKey::TV_INPUT_VGA_1;
    // systemToDavaKeyTranslator[252] = eKeyboardKey::TV_AUDIO_DESCRIPTION;
    // systemToDavaKeyTranslator[253] = eKeyboardKey::TV_AUDIO_DESCRIPTION_MIX_UP;
    // systemToDavaKeyTranslator[254] = eKeyboardKey::TV_AUDIO_DESCRIPTION_MIX_DOWN;
    // systemToDavaKeyTranslator[255] = eKeyboardKey::TV_ZOOM_MODE;
    // systemToDavaKeyTranslator[256] = eKeyboardKey::TV_CONTENTS_MENU;
    // systemToDavaKeyTranslator[257] = eKeyboardKey::TV_MEDIA_CONTEXT_MENU;
    // systemToDavaKeyTranslator[258] = eKeyboardKey::TV_TIMER_PROGRAMMING;
    // systemToDavaKeyTranslator[259] = eKeyboardKey::HELP;
#endif
}

eKeyboardKey SystemKeyToDavaKey(uint32 systemKeyCode)
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
    return eKeyboardKey::UNKNOWN;
}

}