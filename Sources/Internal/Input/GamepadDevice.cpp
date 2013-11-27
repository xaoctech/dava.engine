//
// Created by Yury Drazdouski on 11/26/13.
//

#include "GamepadDevice.h"

namespace DAVA
{

    // Not implemented for macos, win and android. See working iOS implementation in GamepadManager.mm

    GamepadDevice::GamepadDevice()
    { }

    GamepadDevice::~GamepadDevice()
    { }

    eDavaGamepadProfile GamepadDevice::GetProfile()
    {
        return eDavaGamepadProfile_None;
    }

    float32 GamepadDevice::GetElementState(eDavaGamepadElement element)
    {
        return 0;
    }

    void GamepadDevice::ProcessElementStateChange(eDavaGamepadElement element, float32 value)
    {

    }
}