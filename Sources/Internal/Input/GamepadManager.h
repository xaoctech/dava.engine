//
// Created by Yury Drazdouski on 11/2/13.
//

#ifndef __GamepadManager_H_
#define __GamepadManager_H_


#include "Base/BaseObject.h"

namespace DAVA
{
    class GamepadDevice;

    class GamepadManager : public BaseObject
    {
    public:
        GamepadManager();
        GamepadDevice *GetCurrentGamepad();

    private:
#if defined(__DAVAENGINE_IPHONE__)
        void *m_gamepadManagerIOS;
#elif defined(__DAVAENGINE_ANDROID__)
        // not implemented
#endif
    };
}

#endif
