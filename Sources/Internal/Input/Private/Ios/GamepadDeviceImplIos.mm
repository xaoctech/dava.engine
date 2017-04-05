#include "Input/Private/Ios/GamepadDeviceImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "Input/GamepadDevice.h"
#include "Input/InputElements.h"
#include "Input/Private/DigitalElement.h"

namespace DAVA
{
namespace Private
{
GamepadDeviceImpl::GamepadDeviceImpl(GamepadDevice* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadDeviceImpl::Update()
{
    if ([controller extendedGamepad])
    {
        GCExtendedGamepad* gamepad = [controller extendedGamepad];
        ReadExtendedGamepadElements(gamepad);
    }
    else if ([controller gamepad])
    {
        GCGamepad* gamepad = [controller gamepad];
        ReadGamepadElements(gamepad);
    }
}

void GamepadDeviceImpl::ReadExtendedGamepadElements(GCExtendedGamepad* gamepad)
{
    auto handleButton = [this](eInputElements element, bool pressed) {
        uint32 index = element - eInputElements::GAMEPAD_FIRST_BUTTON;
        DigitalInputElement di(gamepadDevice->buttons[index]);
        if (pressed != di.IsPressed())
        {
            gamepadDevice->buttonChangedMask.set(index);
        }
        pressed ? di.Press() : di.Release();
    };

    handleButton(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    handleButton(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    handleButton(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    handleButton(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    handleButton(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    handleButton(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);

    auto handleAxis = [this](eInputElements element, float32 newValue) {
        uint32 index = element - eInputElements::GAMEPAD_FIRST_AXIS;
        if (newValue != gamepadDevice->axises[index].x)
        {
            gamepadDevice->axises[index].x = newValue;
            gamepadDevice->axisChangedMask.set(index);
        }
    };

    handleAxis(eInputElements::GAMEPAD_LTHUMB_X, gamepad.leftThumbstick.xAxis.value);
    handleAxis(eInputElements::GAMEPAD_LTHUMB_Y, gamepad.leftThumbstick.yAxis.value);
    handleAxis(eInputElements::GAMEPAD_RTHUMB_X, gamepad.rightThumbstick.xAxis.value);
    handleAxis(eInputElements::GAMEPAD_RTHUMB_Y, gamepad.rightThumbstick.yAxis.value);
    handleAxis(eInputElements::GAMEPAD_LTRIGGER, gamepad.leftTrigger.value);
    handleAxis(eInputElements::GAMEPAD_RTRIGGER, gamepad.rightTrigger.value);
}

void GamepadDeviceImpl::ReadGamepadElements(GCGamepad* gamepad)
{
    auto handleButton = [this](eInputElements element, bool pressed) {
        uint32 index = element - eInputElements::GAMEPAD_FIRST_BUTTON;
        DigitalInputElement di(gamepadDevice->buttons[index]);
        if (pressed != di.IsPressed())
        {
            gamepadDevice->buttonChangedMask.set(index);
        }
        pressed ? di.Press() : di.Release();
    };

    handleButton(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    handleButton(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    handleButton(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    handleButton(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    handleButton(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    handleButton(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    handleButton(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    if (controller == nullptr)
    {
        NSArray<GCController*>* controllers = [GCController controllers];
        if ([controllers count] != 0)
        {
            controller = (__bridge GCController*)CFBridgingRetain([controllers objectAtIndex:0]);
            gamepadDevice->profile = [controller extendedGamepad] != nil ? eGamepadProfiles::EXTENDED : eGamepadProfiles::SIMPLE;
        }
    }
    return controller != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    bool removed = true;
    for (GCController* c in [GCController controllers])
    {
        if (c == controller)
        {
            removed = false;
            break;
        }
    }
    if (removed && controller != nullptr)
    {
        CFBridgingRelease(controller);
        controller = nullptr;
    }
    return controller != nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
