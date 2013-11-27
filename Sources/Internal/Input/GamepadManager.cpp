//
// Created by Yury Drazdouski on 11/26/13.
//

#include "GamepadManager.h"
#import "GamepadDevice.h"

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