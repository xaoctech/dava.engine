/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/
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
