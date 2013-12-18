//
// Created by Yury Drazdouski on 11/26/13.
//

#include "GamepadManager.h"
#include "GamepadDevice.h"

namespace DAVA
{

    // Not implemented for macos, win and android. See working iOS implementation in GamepadManager.mm

    GamepadManager::GamepadManager()
    { }

    GamepadDevice *GamepadManager::GetCurrentGamepad()
    {
        return NULL;
    }
}