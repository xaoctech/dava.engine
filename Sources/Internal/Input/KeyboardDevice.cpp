/*
 *  KeyboardDevice.cpp
 *  Framework
 *
 *  Created by Alexey Prosin on 1/3/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "KeyboardDevice.h"

namespace DAVA
{

KeyboardDevice::KeyboardDevice()
{
    PrepareKeyTranslator();
}

KeyboardDevice::~KeyboardDevice()
{
}
    
bool KeyboardDevice::IsKeyPressed(int32 keyCode)
{
    DVASSERT(keyCode < DVKEY_COUNT);
    return keyStatus[keyCode];
}

void KeyboardDevice::OnKeyPressed(int32 keyCode)
{
    DVASSERT(keyCode < DVKEY_COUNT);
    keyStatus[keyCode] = true;
    realKeyStatus[keyCode] = true;
}

void KeyboardDevice::OnKeyUnpressed(int32 keyCode)
{
    DVASSERT(keyCode < DVKEY_COUNT);
    realKeyStatus[keyCode] = false;
}

void KeyboardDevice::OnBeforeUpdate()
{
}

void KeyboardDevice::OnAfterUpdate()
{
    for (int i = 0; i < DVKEY_COUNT; i++) 
    {
        keyStatus[i] = realKeyStatus[i];
    }
}

void KeyboardDevice::OnSystemKeyPressed(int32 systemKeyCode)
{
    Logger::Debug("System key pressed 0x%X", systemKeyCode);
    DVASSERT(systemKeyCode < MAX_KEYS);
    OnKeyPressed(keyTranslator[systemKeyCode]);
}

void KeyboardDevice::OnSystemKeyUnpressed(int32 systemKeyCode)
{
    Logger::Debug("System key unpressed 0x%X", systemKeyCode);
    DVASSERT(systemKeyCode < MAX_KEYS);
    OnKeyUnpressed(keyTranslator[systemKeyCode]);
}

    
void KeyboardDevice::PrepareKeyTranslator()
{
    for (int i = 0; i < MAX_KEYS; i++) 
    {
        keyTranslator[i] = DVKEY_UNKNOWN;
    }
#if defined(__DAVAENGINE_WIN32__)
#endif

#if defined(__DAVAENGINE_MACOS__)
    keyTranslator[0x7B] = DVKEY_LEFT;
    keyTranslator[0x7C] = DVKEY_RIGHT;
    keyTranslator[0x7D] = DVKEY_UP;
    keyTranslator[0x7E] = DVKEY_DOWN;
    keyTranslator[DVMACOS_COMMAND] = DVKEY_CTRL;
    keyTranslator[DVMACOS_OPTION] = DVKEY_ALT;
    keyTranslator[DVMACOS_SHIFT] = DVKEY_SHIFT;
    keyTranslator[DVMACOS_CAPS_LOCK] = DVKEY_CAPSLOCK;
#endif
}
    

};
