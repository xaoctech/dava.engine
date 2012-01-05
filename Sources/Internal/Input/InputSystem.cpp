/*
 *  InputSystem.cpp
 *  Framework
 *
 *  Created by Alexey Prosin on 1/3/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "InputSystem.h"
#include "Input/KeyboardDevice.h"

namespace DAVA 
{

InputSystem::InputSystem()
{
    keyboard = new KeyboardDevice();
}
    
InputSystem::~InputSystem()
{
    SafeRelease(keyboard);
}

void InputSystem::OnBeforeUpdate()
{
    keyboard->OnBeforeUpdate();
}
    
void InputSystem::OnAfterUpdate()
{
    keyboard->OnAfterUpdate();
}
    
KeyboardDevice *InputSystem::GetKeyboard()
{
    return keyboard;
}
    
};
