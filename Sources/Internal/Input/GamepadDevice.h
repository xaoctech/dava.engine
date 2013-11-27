//
// Created by Yury Drazdouski on 11/20/13.
//

#ifndef __GamepadDevice_H_
#define __GamepadDevice_H_

#include "Base/BaseObject.h"
#include "UI/UIEvent.h"

namespace DAVA
{
    class GamepadDevice;

    enum eDavaGamepadProfile
    {
        eDavaGamepadProfile_None,              // No device
        eDavaGamepadProfile_SimpleProfile,     // Logitech for iPhone 5/5s/5c
        eDavaGamepadProfile_ExtendedProfile    // Moga for iPhone 5/5s/5c, Moga for Android
    };


    enum eDavaGamepadElement
    {
        eDavaGamepadElement_LShoulderButton,
        eDavaGamepadElement_RShoulderButton,
        eDavaGamepadElement_LTrigger,
        eDavaGamepadElement_RTrigger,
        eDavaGamepadElement_ButtonA,
        eDavaGamepadElement_ButtonB,
        eDavaGamepadElement_ButtonX,
        eDavaGamepadElement_ButtonY,
        eDavaGamepadElement_LThumbstickX,
        eDavaGamepadElement_LThumbstickY,
        eDavaGamepadElement_RThumbstickX,
        eDavaGamepadElement_RThumbstickY,
        eDavaGamepadElement_DPadX,
        eDavaGamepadElement_DPadY
    };


    class GamepadDevice : public BaseObject
    {
    public:
#if defined(__DAVAENGINE_IPHONE__)
        GamepadDevice(void *gameController);
#elif defined(__DAVAENGINE_ANDROID__)
        GamepadDevice();
#else
        GamepadDevice();
#endif
        ~GamepadDevice();
        eDavaGamepadProfile GetProfile();
        float32 GetElementState(eDavaGamepadElement element);

    private:
        void ProcessElementStateChange(eDavaGamepadElement element, float32 value);
#if defined(__DAVAENGINE_IPHONE__)
        void *m_gameController;
#elif defined(__DAVAENGINE_ANDROID__)
        // not implemented
#endif
    };

}

#endif //__GamepadDevice_H_
